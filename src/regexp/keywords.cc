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

namespace {

constexpr uint32_t MAX_KEYWORD_LENGTH = 16;
constexpr uint32_t MAX_KEYWORDS = 0xFFFE;
constexpr uint32_t MAX_TRIES = 4096;

using posset_t = uint32_t; // bit `i` set if position `i` in the keyword is reachable

class Matcher {
    const opt_t* opts;
    const Enc& enc;
    IrAllocator alc;
    RangeMgr rm;
    std::map<const AstNode*, const Range*> charsets;

    const std::vector<uint32_t>* str; // keyword code units
    bool unsupported;

  public:
    explicit Matcher(const opt_t* opts)
        : opts(opts), enc(opts->encoding), alc(), rm(alc), charsets(), str(nullptr)
        , unsupported(false) {}

    bool failed() const { return unsupported; }

    // Returns true if the whole string `s` is in the language of `ast`.
    bool match(const AstNode* ast, const std::vector<uint32_t>& s) {
        str = &s;
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

    // Positions reachable after one code unit that is in the given class. An empty class behaves
    // according to the `re2c:empty-class` policy, see `re_class` in ast_to_re.cc.
    posset_t step_class(const Range* r, posset_t from) {
        if (r == nullptr) {
            return opts->empty_class == EmptyClass::MATCH_EMPTY ? from : 0;
        }
        posset_t to = 0;
        for (size_t p = 0; p < str->size(); ++p) {
            if (((from >> p) & 1u) && contains(r, (*str)[p])) to |= 1u << (p + 1);
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
                    uint32_t c = ast->str.chars[j].chr, d = (*str)[p + j];
                    ok = icase ? (d == enc.to_lower(c) || d == enc.to_upper(c) || d == c) : d == c;
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
// implicit group that the parser adds around every rule), otherwise an empty vector.
std::vector<uint32_t> literal(const opt_t* opts, const AstNode* ast) {
    std::vector<uint32_t> s;
    while (ast->kind == AstKind::CAP) ast = ast->cap.ast;
    if (ast->kind != AstKind::STR) return s;

    const Enc& enc = opts->encoding;
    const bool icase = opts->case_insensitive || ast->str.icase != opts->case_inverted;
    const uint32_t maxchr = enc.type() == Enc::Type::UTF8 ? 0x80 : 0x100;
    for (const AstChar& c : ast->str.chars) {
        if (c.chr >= maxchr) return {};
        if (icase && (enc.to_lower(c.chr) != c.chr || enc.to_upper(c.chr) != c.chr)) return {};
        s.push_back(c.chr);
    }
    if (s.size() > MAX_KEYWORD_LENGTH) return {};
    return s;
}

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

    // For every candidate literal find its host rule (if any).
    Matcher matcher(opts);
    std::vector<size_t> host(nrules, Rule::NONE);
    std::vector<std::vector<uint32_t>> strings(nrules);
    for (size_t k = 0; k < nrules; ++k) {
        if (is_special[k]) continue;
        std::vector<uint32_t> s = literal(opts, ast[k].ast);
        if (s.empty()) continue;

        // The literal must be the first rule matching its string, and the host is the next one.
        size_t w = Rule::NONE;
        bool shadowed = false;
        for (size_t i = 0; i < nrules; ++i) {
            if (i == k || !matcher.match(ast[i].ast, s)) continue;
            if (i < k) { shadowed = true; break; }
            w = i;
            break;
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
            w |= static_cast<uint64_t>(strings[i][j]) << (8 * (j % 8));
        }
        t->keys.push_back(kw);
        ++removed;
    }
    if (!find_perfect_hash(*t)) {
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
