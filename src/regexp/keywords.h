#ifndef _RE2C_REGEXP_KEYWORDS_
#define _RE2C_REGEXP_KEYWORDS_

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include "src/parse/ast.h"
#include "src/util/forbid_copy.h"

namespace re2c {

class Msg;
struct opt_t;
struct SemAct;

// A keyword that has been removed from the DFA and is recognized by a perfect hash lookup in the
// semantic action of its host rule instead.
struct Keyword {
    uint64_t word0;        // code units 0..7 of the keyword, little-endian, zero-padded
    uint64_t word1;        // code units 8..15 of the keyword, little-endian, zero-padded
    uint32_t length;       // number of code units (1..16)
    const SemAct* semact;  // semantic action of the removed keyword rule
};

// Flat composite table in the layout of `c2_kw_table_t` (see c2simd `c2_kw_resolve.h`), used with
// `re2c:keywords:model = archtime`, see note [archtime keyword tables].
struct ArchtimeTable {
    static constexpr uint32_t SLOTS = 256;   // number of 16-byte key slots
    static constexpr uint32_t BUCKETS = 64;  // number of first-level buckets
    static constexpr uint32_t KEYSIZE = 16;  // size of a key slot in bytes
    static constexpr uint8_t FOLD_ASCII = 0x20;

    uint64_t m1;               // first-level hash multiplier (bucket)
    uint64_t m2;               // second-level hash multiplier (slot)
    std::vector<uint16_t> eh0; // bit `n` set if a key of length `n` < 16 starts with this byte
    std::vector<uint8_t> disp; // displacement of each bucket
    std::vector<uint8_t> key;  // 16-byte zero-padded keys, folded to lower case if `fold` is set
    std::vector<uint8_t> len;  // key length in each slot, 0 for an empty slot
    std::vector<uint16_t> tok; // `1 + index` of the keyword in each slot, 0 for an empty slot
    uint8_t fold;              // FOLD_ASCII if input bytes 'A'..'Z' are folded, 0 otherwise

    ArchtimeTable()
        : m1(0), m2(0), eh0(256, 0), disp(BUCKETS, 0), key(SLOTS * KEYSIZE, 0), len(SLOTS, 0)
        , tok(SLOTS, 0), fold(0) {}
};

// See note [keyword tables].
struct KeywordTable {
    std::string name;            // name of the lookup function and prefix of the tables
    size_t host_rule;            // index of the host rule among the rules left in the DFA
    std::vector<Keyword> keys;   // keyword `i` is reported by the lookup as `i + 1`
    uint32_t bits;               // log2 of the number of slots
    uint64_t mul0;               // hash multiplier for the first word
    uint64_t mul1;               // hash multiplier for the second word
    std::vector<uint32_t> slots; // 0 for an empty slot, otherwise `1 + index` in `keys`
    std::unique_ptr<ArchtimeTable> archtime; // non-null with `re2c:keywords:model = archtime`
    mutable bool emitted;        // whether a `keywords` directive has emitted this table

    KeywordTable()
        : name(), host_rule(0), keys(), bits(0), mul0(0), mul1(0), slots(), archtime()
        , emitted(false) {}
    FORBID_COPY(KeywordTable);
};

// Find keyword rules that can be moved out of the DFA of the given grammar into a keyword table.
// On success `rules` and `def_rule` describe the reduced grammar and `table` holds the table;
// otherwise `table` is null and `rules` is a copy of the original rules.
void find_keywords(const opt_t* opts,
                   Msg& msg,
                   const AstGram& gram,
                   const loc_t& loc,
                   std::vector<AstRule>& rules,
                   size_t& def_rule,
                   std::unique_ptr<KeywordTable>& table);

} // namespace re2c

#endif // _RE2C_REGEXP_KEYWORDS_
