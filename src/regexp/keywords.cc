#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/encoding/enc.h"
#include "src/msg/msg.h"
#include "src/msg/warn.h"
#include "src/options/opt.h"
#include "src/parse/ast.h"
#include "src/regexp/keywords.h"
#include "src/regexp/rule.h"
#include "src/util/allocator.h"
#include "src/util/forbid_copy.h"
#include "src/util/range.h"

namespace re2c {

// note [keyword tables]
//
// With `re2c:keywords` enabled, literal rules that are fully subsumed by a later rule (typically
// keywords subsumed by an identifier rule) are removed from the DFA. The later rule (the host)
// matches the same tokens in their place, and its semantic action classifies the matched token
// with a perfect hash table computed at generation time, dispatching to the semantic action of
// the removed keyword rule on a hit.
//
// For a literal rule K with string k, let M(k) be the set of rules whose regular expression
// (including trailing context, as the longest-match competition is on the full match length)
// matches k. K is moved to the table of host W iff:
//
//   - K is a case-sensitive literal of 1 to 16 single-byte code units without tags;
//   - K has the highest priority in M(k) (otherwise K is shadowed and left alone);
//   - W has the highest priority in M(k) \ {K}, has no tags, no trailing context, and is not a
//     special rule (default rule `*` or end-of-input rule `$`).
//
// Removing K does not change the set of match lengths at any input position, since whenever K
// matches k, W matches k as well. So the longest match is unchanged. If the matched text is not
// k, K does not take part and the winner is unchanged. If it is k, the original winner is K and
// the new winner is W, whose semantic action looks up the matched text (which is k) and runs the
// action of K. Therefore the observable behaviour of the lexer is preserved.
//
// Membership of k in the language of a rule is decided exactly on the AST by propagating the
// set of reachable positions in k (a bitmask, as |k| <= 16), with character classes evaluated by
// the same range operations as in the conversion of AST to regexp.

// note [archtime keyword tables]
//
// With `re2c:keywords:model = archtime` the table has the flat composite layout `c2_kw_table_t`
// shared with c2simd (`sources/c2archtsim/c2_kw_resolve.h`, schema `sgoiter/spec/keyword.cue`):
// a 16-bit mask of key lengths per leading byte (level 0, a one-load rejection of most
// identifiers), a two-level perfect hash (bucket `(w * m1) >> 58`, slot `((w * m2) >> 56) ^
// disp[bucket]`) over 256 slots of 16-byte zero-padded keys, where the hash word `w` is the first
// min(n, 8) bytes of the key XOR `n << 56`. The search of multipliers and displacements follows
// c2simd `cmd/c2kwgen`. The model has three extra constraints: at most 256 keys, pairwise distinct
// hash words (a key whose hash word is already taken stays in the DFA), and a single case mode.
//
// The table may fold input bytes 'A'..'Z' to lower case (field `fold`). Folding is only sound if
// every keyword K in the table is case-insensitive: then K matches every case variant v of k, and
// the conditions of note [keyword tables] must hold for all variants at once. The matcher decides
// this on sets of code units per position: a rule matches *some* variant if a path accepts at each
// position one of the two cases (exact, as a path consumes every position once), and a rule matches
// *every* variant if a path accepts both cases at each position (a sufficient condition). K goes to
// the table of W iff no other rule than W after K and no rule before K matches some variant, and W
// matches every variant. Case-sensitive keywords with letters cannot share a folded table, so the
// case mode of the larger group wins and the other group stays in the DFA.

namespace {

constexpr uint32_t MAX_KEYWORD_LENGTH = 16;
constexpr uint32_t MAX_KEYWORDS = 0xFFFE;
constexpr uint32_t MAX_TRIES = 4096;

using posset_t = uint32_t; // bit `i` set if position `i` in the keyword is reachable

// Code units accepted at one position of a keyword: `lo == up` for a case-sensitive position,
// otherwise the lower and upper case of an ASCII letter in a case-insensitive keyword.
struct KeyChar {
    uint32_t lo;
    uint32_t up;
};
using keystr_t = std::vector<KeyChar>;

// Whether a rule must match some or every case variant of a keyword, see note [archtime keyword
// tables]. Both are the same for a case-sensitive keyword.
enum class Quant { ANY, ALL };

class Matcher {
    const opt_t* opts;
    const Enc& enc;
    IrAllocator alc;
    RangeMgr rm;
    std::map<const AstNode*, const Range*> charsets;

    const keystr_t* str; // keyword code units
    Quant quant;
    bool unsupported;

  public:
    explicit Matcher(const opt_t* opts)
        : opts(opts), enc(opts->encoding), alc(), rm(alc), charsets(), str(nullptr)
        , quant(Quant::ANY), unsupported(false) {}

    bool failed() const { return unsupported; }

    // Returns true if some (Quant::ANY) or every (Quant::ALL) case variant of the whole string `s`
    // is in the language of `ast`.
    bool match(const AstNode* ast, const keystr_t& s, Quant q) {
        str = &s;
        quant = q;
        return (ends(ast, 1u) >> s.size()) & 1u;
    }

  private:
    bool is_icase(bool icase) const {
        return opts->case_insensitive || icase != opts->case_inverted;
    }

    // Character class denoted by a node, or nullptr for an empty class. Mirrors `cls_to_range`,
    // `dot_to_range`, `char_to_range` and `diff_to_range` in ast_to_re.cc.
    const Range* charset(const AstNode* ast) {
        auto i = charsets.find(ast);
        if (i != charsets.end()) return i->second;

        Range* r = nullptr;
        switch (ast->kind) {
        case AstKind::CLS:
            for (const AstRange& a : ast->cls.ranges) {
                Range* s = enc.validate_range(rm, a.lower, a.upper);
                if (s == nullptr) { unsupported = true; break; }
                r = rm.add(r, s);
            }
            if (ast->cls.negated) r = rm.sub(enc.full_range(rm), r);
            break;
        case AstKind::DOT:
            r = rm.sub(enc.full_range(rm), rm.sym('\n'));
            break;
        case AstKind::STR: {
            if (ast->str.chars.size() != 1) { unsupported = true; break; }
            uint32_t c = ast->str.chars[0].chr;
            if (is_icase(ast->str.icase)) {
                uint32_t l = enc.to_lower(c), u = enc.to_upper(c);
                r = (l != u) ? rm.add(rm.sym(l), rm.sym(u)) : rm.sym(c);
            } else {
                r = rm.sym(c);
            }
            break;
        }
        case AstKind::CAP:
            r = rm.add(nullptr, charset(ast->cap.ast));
            break;
        case AstKind::ALT:
            r = rm.add(charset(ast->alt.ast1), charset(ast->alt.ast2));
            break;
        case AstKind::DIFF:
            r = rm.sub(charset(ast->diff.ast1), charset(ast->diff.ast2));
            break;
        default:
            unsupported = true;
            break;
        }
        charsets[ast] = r;
        return r;
    }

    static bool contains(const Range* r, uint32_t c) {
        for (; r != nullptr; r = r->next()) {
            if (c >= r->lower() && c < r->upper()) return true;
        }
        return false;
    }

    // Combine the test of both cases at a keyword position according to the quantifier.
    bool accepts(bool lo, bool up) const {
        return quant == Quant::ANY ? lo || up : lo && up;
    }

    // Positions reachable after one code unit that is in the given class. An empty class behaves
    // according to the `re2c:empty-class` policy, see `re_class` in ast_to_re.cc.
    posset_t step_class(const Range* r, posset_t from) {
        if (r == nullptr) {
            return opts->empty_class == EmptyClass::MATCH_EMPTY ? from : 0;
        }
        posset_t to = 0;
        for (size_t p = 0; p < str->size(); ++p) {
            const KeyChar& k = (*str)[p];
            if (((from >> p) & 1u) && accepts(contains(r, k.lo), contains(r, k.up))) {
                to |= 1u << (p + 1);
            }
        }
        return to;
    }

    posset_t ends(const AstNode* ast, posset_t from) {
        if (from == 0 || unsupported) return 0;

        switch (ast->kind) {
        case AstKind::NIL:
        case AstKind::TAG:
            return from;
        case AstKind::END:
            // End-of-input symbol never occurs inside the keyword.
            return 0;
        case AstKind::DEF: {
            posset_t to = 0;
            for (size_t p = 0; p < str->size(); ++p) {
                if ((from >> p) & 1u) to |= 1u << (p + 1);
            }
            return to;
        }
        case AstKind::CLS:
        case AstKind::DOT:
        case AstKind::DIFF:
            return step_class(charset(ast), from);
        case AstKind::STR: {
            const bool icase = is_icase(ast->str.icase);
            const size_t n = ast->str.chars.size();
            posset_t to = 0;
            for (size_t p = 0; p + n <= str->size(); ++p) {
                if (((from >> p) & 1u) == 0) continue;
                bool ok = true;
                for (size_t j = 0; ok && j < n; ++j) {
                    const uint32_t c = ast->str.chars[j].chr;
                    const KeyChar& k = (*str)[p + j];
                    auto eq = [&](uint32_t d) {
                        return icase ? (d == enc.to_lower(c) || d == enc.to_upper(c) || d == c)
                                     : d == c;
                    };
                    ok = accepts(eq(k.lo), eq(k.up));
                }
                if (ok) to |= 1u << (p + n);
            }
            return to;
        }
        case AstKind::ALT:
            return ends(ast->alt.ast1, from) | ends(ast->alt.ast2, from);
        case AstKind::CAT:
            return ends(ast->cat.ast2, ends(ast->cat.ast1, from));
        case AstKind::CAP:
            return ends(ast->cap.ast, from);
        case AstKind::ITER: {
            const uint32_t min = ast->iter.min, max = ast->iter.max;
            posset_t cur = from;
            // Mandatory iterations. The sequence of sets is deterministic, so a repeated set means
            // a fixpoint.
            for (uint32_t i = 0; i < min && cur != 0; ++i) {
                posset_t next = ends(ast->iter.ast, cur);
                if (next == cur) break;
                cur = next;
            }
            // Optional iterations. An iteration that consumes nothing can be dropped, so no more
            // than |k| optional iterations are needed.
            posset_t acc = cur;
            for (uint32_t i = min; i < max && i - min <= MAX_KEYWORD_LENGTH && cur != 0; ++i) {
                cur = ends(ast->iter.ast, cur);
                if ((cur & ~acc) == 0) break;
                acc |= cur;
            }
            return acc;
        }
        }
        unsupported = true;
        return 0;
    }

    FORBID_COPY(Matcher);
};

bool has_tags(const AstNode* ast) {
    switch (ast->kind) {
    case AstKind::TAG:
        return true;
    case AstKind::ALT:
        return has_tags(ast->alt.ast1) || has_tags(ast->alt.ast2);
    case AstKind::CAT:
        return has_tags(ast->cat.ast1) || has_tags(ast->cat.ast2);
    case AstKind::DIFF:
        return has_tags(ast->diff.ast1) || has_tags(ast->diff.ast2);
    case AstKind::ITER:
        return has_tags(ast->iter.ast);
    case AstKind::CAP:
        return has_tags(ast->cap.ast);
    default:
        return false;
    }
}

// Return the keyword string if the rule is a plain case-sensitive literal (possibly wrapped in the
// implicit group that the parser adds around every rule), otherwise an empty vector. If
// `allow_icase` is set, a case-insensitive literal is accepted as well provided that its only cased
// code units are ASCII letters (see note [archtime keyword tables]).
keystr_t literal(const opt_t* opts, const AstNode* ast, bool allow_icase) {
    keystr_t s;
    while (ast->kind == AstKind::CAP) ast = ast->cap.ast;
    if (ast->kind != AstKind::STR) return s;

    const Enc& enc = opts->encoding;
    const bool icase = opts->case_insensitive || ast->str.icase != opts->case_inverted;
    const uint32_t maxchr = enc.type() == Enc::Type::UTF8 ? 0x80 : 0x100;
    for (const AstChar& c : ast->str.chars) {
        if (c.chr >= maxchr) return {};
        const uint32_t lo = enc.to_lower(c.chr), up = enc.to_upper(c.chr);
        if (!icase || (lo == c.chr && up == c.chr)) {
            s.push_back({c.chr, c.chr});
        } else if (allow_icase && lo >= 'a' && lo <= 'z' && up == lo - 0x20) {
            s.push_back({lo, up});
        } else {
            return {};
        }
    }
    if (s.size() > MAX_KEYWORD_LENGTH) return {};
    return s;
}

bool is_ascii_letter(uint32_t c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Case mode of a keyword with respect to a folded archtime table.
enum class CaseMode {
    NEUTRAL,   // no letters: the same with or without folding
    SENSITIVE, // case-sensitive letters: requires a table without folding
    FOLDED     // case-insensitive letters: requires a folded table
};

CaseMode case_mode(const keystr_t& s) {
    CaseMode m = CaseMode::NEUTRAL;
    for (const KeyChar& k : s) {
        // A literal is either case-sensitive or case-insensitive as a whole.
        if (k.lo != k.up) return CaseMode::FOLDED;
        if (is_ascii_letter(k.lo)) m = CaseMode::SENSITIVE;
    }
    return m;
}

// The code unit stored in the table for a keyword position (lower case for a folded one).
inline uint32_t key_unit(const KeyChar& k) { return k.lo; }

uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

inline uint32_t slot_of(const Keyword& k, uint64_t mul0, uint64_t mul1, uint32_t bits) {
    return static_cast<uint32_t>((k.word0 * mul0 + k.word1 * mul1) >> (64 - bits));
}

// Deterministic bounded search for a collision-free multiply-shift hash, see note [keyword tables].
bool find_perfect_hash(KeywordTable& t) {
    const size_t n = t.keys.size();
    uint32_t bits = 4;
    while ((size_t{1} << bits) < 2 * n) ++bits;
    const size_t maxsize = std::max<size_t>(1024, 16 * n);

    uint64_t seed = 0x2545F4914F6CDD1Dull;
    std::vector<uint32_t> slots;
    for (; bits <= 16 && (size_t{1} << bits) <= maxsize; ++bits) {
        for (uint32_t i = 0; i < MAX_TRIES; ++i) {
            const uint64_t mul0 = splitmix64(seed) | 1u, mul1 = splitmix64(seed) | 1u;
            slots.assign(size_t{1} << bits, 0);
            bool ok = true;
            for (size_t k = 0; ok && k < n; ++k) {
                uint32_t& s = slots[slot_of(t.keys[k], mul0, mul1, bits)];
                if (s != 0) ok = false;
                s = static_cast<uint32_t>(k + 1);
            }
            if (ok) {
                t.bits = bits;
                t.mul0 = mul0;
                t.mul1 = mul1;
                t.slots.swap(slots);
                return true;
            }
        }
    }
    return false;
}

// Deterministic generator of candidate multipliers, the same as in c2simd `cmd/c2kwgen`.
struct Lcg64 {
    uint64_t state;
    uint64_t next() {
        state = state * 6364136223846793005ull + 1442695040888963407ull;
        return state;
    }
};

inline uint64_t archtime_hash_word(const Keyword& k) {
    return k.word0 ^ (static_cast<uint64_t>(k.length) << 56);
}

// Search for multipliers and bucket displacements of a collision-free two-level hash, see note
// [archtime keyword tables]. The hash words of the keys must be pairwise distinct.
bool find_archtime_hash(const std::vector<Keyword>& keys, uint8_t fold, ArchtimeTable& t) {
    constexpr uint32_t MAX_ATTEMPTS = 100000;
    const size_t n = keys.size();
    if (n > ArchtimeTable::SLOTS) return false;

    std::vector<uint64_t> words(n);
    for (size_t i = 0; i < n; ++i) words[i] = archtime_hash_word(keys[i]);

    Lcg64 rng = {0x9E3779B97F4A7C15ull};
    std::vector<std::vector<size_t>> buckets(ArchtimeTable::BUCKETS);
    std::vector<size_t> order(ArchtimeTable::BUCKETS);
    std::vector<uint32_t> slots(n), tmp;
    for (uint32_t attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        const uint64_t m1 = rng.next() | 1u, m2 = rng.next() | 1u;

        for (std::vector<size_t>& b : buckets) b.clear();
        for (size_t i = 0; i < n; ++i) buckets[(words[i] * m1) >> 58].push_back(i);

        // Place the largest buckets first (ties in bucket order, so the search is deterministic).
        for (size_t b = 0; b < order.size(); ++b) order[b] = b;
        std::stable_sort(order.begin(), order.end(), [&](size_t x, size_t y) {
            return buckets[x].size() > buckets[y].size();
        });

        std::vector<uint8_t> disp(ArchtimeTable::BUCKETS, 0);
        std::vector<bool> occupied(ArchtimeTable::SLOTS, false);
        bool ok = true;
        for (size_t b : order) {
            if (buckets[b].empty()) continue;
            bool found = false;
            for (uint32_t d = 0; !found && d < 256; ++d) {
                tmp.clear();
                bool conflict = false;
                for (size_t i : buckets[b]) {
                    const uint32_t s = (static_cast<uint32_t>((words[i] * m2) >> 56) ^ d) & 0xFF;
                    if (occupied[s] || std::find(tmp.begin(), tmp.end(), s) != tmp.end()) {
                        conflict = true;
                        break;
                    }
                    tmp.push_back(s);
                }
                if (conflict) continue;
                disp[b] = static_cast<uint8_t>(d);
                for (size_t j = 0; j < tmp.size(); ++j) {
                    occupied[tmp[j]] = true;
                    slots[buckets[b][j]] = tmp[j];
                }
                found = true;
            }
            if (!found) { ok = false; break; }
        }
        if (!ok) continue;

        t.m1 = m1;
        t.m2 = m2;
        t.disp.swap(disp);
        t.fold = fold;
        for (size_t i = 0; i < n; ++i) {
            const Keyword& k = keys[i];
            const uint32_t s = slots[i];
            for (uint32_t j = 0; j < k.length; ++j) {
                const uint64_t w = j < 8 ? k.word0 : k.word1;
                t.key[s * ArchtimeTable::KEYSIZE + j] = static_cast<uint8_t>(w >> (8 * (j % 8)));
            }
            t.len[s] = static_cast<uint8_t>(k.length);
            t.tok[s] = static_cast<uint16_t>(i + 1);
            // Level 0: the leading byte is tested before folding, so both cases open the length.
            if (k.length < 16) {
                const uint8_t c = static_cast<uint8_t>(k.word0);
                const uint16_t bit = static_cast<uint16_t>(1u << k.length);
                t.eh0[c] |= bit;
                if (fold && c >= 'a' && c <= 'z') t.eh0[c - 0x20] |= bit;
            }
        }
        return true;
    }
    return false;
}

} // anonymous namespace

void find_keywords(const opt_t* opts,
                   Msg& msg,
                   const AstGram& gram,
                   const loc_t& loc,
                   std::vector<AstRule>& rules,
                   size_t& def_rule,
                   std::unique_ptr<KeywordTable>& table) {
    const std::vector<AstRule>& ast = gram.rules;
    rules = ast;
    def_rule = gram.def_rule;
    table.reset();

    const Enc::Type enc = opts->encoding.type();
    if (enc != Enc::Type::ASCII && enc != Enc::Type::UTF8) {
        msg.warn.keyword_table(loc, gram.name, "encoding with multi-byte code units or EBCDIC");
        return;
    }

    if (opts->captures) {
        msg.warn.keyword_table(loc, gram.name, "capturing groups are enabled");
        return;
    }

    const size_t nrules = ast.size();
    std::vector<bool> is_special(nrules, false);
    std::vector<bool> tagged(nrules, false);
    for (size_t i = 0; i < nrules; ++i) {
        const AstNode* a = ast[i].ast;
        is_special[i] = i == gram.def_rule || is_oldstyle_eof(a) || a->kind == AstKind::DEF;
        tagged[i] = has_tags(a);
    }

    const bool archtime = opts->keywords_model == KeywordsModel::ARCHTIME;

    // For every candidate literal find its host rule (if any).
    Matcher matcher(opts);
    std::vector<size_t> host(nrules, Rule::NONE);
    std::vector<keystr_t> strings(nrules);
    for (size_t k = 0; k < nrules; ++k) {
        if (is_special[k]) continue;
        keystr_t s = literal(opts, ast[k].ast, archtime);
        if (s.empty()) continue;

        // The literal must be the first rule matching its string, and the host is the next one;
        // for a case-insensitive literal the host must also match every case variant.
        size_t w = Rule::NONE;
        bool shadowed = false;
        for (size_t i = 0; i < nrules; ++i) {
            if (i == k || !matcher.match(ast[i].ast, s, Quant::ANY)) continue;
            if (i < k) { shadowed = true; break; }
            w = i;
            break;
        }
        if (w != Rule::NONE && !shadowed && !matcher.match(ast[w].ast, s, Quant::ALL)) {
            w = Rule::NONE;
        }
        if (matcher.failed()) {
            msg.warn.keyword_table(loc, gram.name, "unsupported regular expression construct");
            return;
        }
        if (shadowed || w == Rule::NONE || is_special[w] || tagged[w]) continue;

        host[k] = w;
        strings[k].swap(s);
    }

    // Choose the host with the largest number of keywords (the first one on a tie).
    std::map<size_t, size_t> count;
    for (size_t k = 0; k < nrules; ++k) {
        if (host[k] != Rule::NONE) ++count[host[k]];
    }
    size_t best = Rule::NONE, nbest = 0;
    for (const auto& c : count) {
        if (c.second > nbest) { best = c.first; nbest = c.second; }
    }
    if (best == Rule::NONE) {
        msg.warn.keyword_table(loc, gram.name, "no literal rule is subsumed by a later rule");
        return;
    }
    if (nbest > MAX_KEYWORDS) {
        msg.warn.keyword_table(loc, gram.name, "too many keywords");
        return;
    }

    // An archtime table has a single case mode, at most 256 keys and distinct hash words; the
    // keywords that do not fit stay in the DFA (see note [archtime keyword tables]).
    uint8_t fold = 0;
    if (archtime) {
        size_t nfolded = 0, nsensitive = 0;
        for (size_t k = 0; k < nrules; ++k) {
            if (host[k] != best) continue;
            const CaseMode m = case_mode(strings[k]);
            if (m == CaseMode::FOLDED) ++nfolded;
            if (m == CaseMode::SENSITIVE) ++nsensitive;
        }
        const CaseMode excluded = nfolded > nsensitive ? CaseMode::SENSITIVE : CaseMode::FOLDED;
        if (nfolded > nsensitive) fold = ArchtimeTable::FOLD_ASCII;

        std::vector<uint64_t> seen;
        for (size_t k = 0; k < nrules; ++k) {
            if (host[k] != best) continue;
            uint64_t w0 = 0;
            for (size_t j = 0; j < strings[k].size() && j < 8; ++j) {
                w0 |= static_cast<uint64_t>(key_unit(strings[k][j])) << (8 * j);
            }
            const uint64_t w = w0 ^ (static_cast<uint64_t>(strings[k].size()) << 56);
            if (case_mode(strings[k]) == CaseMode::FOLDED && excluded == CaseMode::FOLDED) {
                host[k] = Rule::NONE;
            } else if (case_mode(strings[k]) == CaseMode::SENSITIVE
                    && excluded == CaseMode::SENSITIVE) {
                host[k] = Rule::NONE;
            } else if (seen.size() >= ArchtimeTable::SLOTS
                    || std::find(seen.begin(), seen.end(), w) != seen.end()) {
                host[k] = Rule::NONE;
            } else {
                seen.push_back(w);
            }
        }
    }

    std::unique_ptr<KeywordTable> t(new KeywordTable());
    std::vector<AstRule> reduced;
    size_t removed = 0;
    for (size_t i = 0; i < nrules; ++i) {
        if (host[i] != best) {
            reduced.push_back(ast[i]);
            continue;
        }
        Keyword kw = {0, 0, static_cast<uint32_t>(strings[i].size()), ast[i].semact};
        for (size_t j = 0; j < strings[i].size(); ++j) {
            uint64_t& w = j < 8 ? kw.word0 : kw.word1;
            w |= static_cast<uint64_t>(key_unit(strings[i][j])) << (8 * (j % 8));
        }
        t->keys.push_back(kw);
        ++removed;
    }
    if (archtime) {
        t->archtime.reset(new ArchtimeTable());
        if (!find_archtime_hash(t->keys, fold, *t->archtime)) {
            msg.warn.keyword_table(loc, gram.name, "no perfect hash function found");
            return;
        }
    } else if (!find_perfect_hash(*t)) {
        msg.warn.keyword_table(loc, gram.name, "no perfect hash function found");
        return;
    }

    // The action of a keyword rule moved from the `<*>` condition is still used.
    for (const Keyword& k : t->keys) k.semact->is_used = true;

    t->host_rule = best - removed; // all removed rules precede the host
    if (def_rule != Rule::NONE) def_rule -= removed; // the default rule is the last one
    rules.swap(reduced);
    table.swap(t);
}

} // namespace re2c
