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

// See note [keyword tables].
struct KeywordTable {
    std::string name;            // name of the lookup function and prefix of the tables
    size_t host_rule;            // index of the host rule among the rules left in the DFA
    std::vector<Keyword> keys;   // keyword `i` is reported by the lookup as `i + 1`
    uint32_t bits;               // log2 of the number of slots
    uint64_t mul0;               // hash multiplier for the first word
    uint64_t mul1;               // hash multiplier for the second word
    std::vector<uint32_t> slots; // 0 for an empty slot, otherwise `1 + index` in `keys`
    mutable bool emitted;        // whether a `keywords` directive has emitted this table

    KeywordTable()
        : name(), host_rule(0), keys(), bits(0), mul0(0), mul1(0), slots(), emitted(false) {}
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
