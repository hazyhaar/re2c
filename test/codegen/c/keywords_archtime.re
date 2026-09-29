// re2c $INPUT -o $OUTPUT -i
#include <assert.h>
#include <stddef.h>
#include <string.h>

// Flat composite keyword tables `c2_kw_table_t` (`re2c:keywords:model = archtime`).
/*!keywords:re2c*/

enum { END, ID, NUM, SPACE, ERR, KW_IF, KW_ELSE, KW_WHILE, KW_RETURN, KW_CONTINUE_A,
    KW_CONTINUE_B, KW_THREAD_LOCAL_VAR, KW_PLUS, KW_SELECT, KW_FROM, KW_WHERE, KW_NULL,
    KW_ORDER_BY };

// Case-sensitive keywords: no folding. `continue_b` has the same hash word as `continue_a`
// (same length and first 8 bytes), so it stays in the DFA; `thread_local_var` has 16 bytes and
// bypasses level 0; `++` is not a keyword of the table, as `[a-z_]` does not subsume it.
static int lex_c(const char** pcur) {
    const char* YYCURSOR = *pcur;
    const char* YYMARKER;
    const char* tok = YYCURSOR;
    int kind;
    (void)YYMARKER;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = char;
    re2c:keywords = 1;
    re2c:keywords:model = archtime;
    re2c:keywords:token = tok;

    "\x00"               { kind = END; goto done; }
    [ ]+                 { kind = SPACE; goto done; }
    [0-9]+               { kind = NUM; goto done; }
    "if"                 { kind = KW_IF; goto done; }
    "else"               { kind = KW_ELSE; goto done; }
    "while"              { kind = KW_WHILE; goto done; }
    "return"             { kind = KW_RETURN; goto done; }
    "continue_a"         { kind = KW_CONTINUE_A; goto done; }
    "continue_b"         { kind = KW_CONTINUE_B; goto done; }
    "thread_local_var"   { kind = KW_THREAD_LOCAL_VAR; goto done; }
    "++"                 { kind = KW_PLUS; goto done; }
    [a-z_][a-z0-9_]*     { kind = ID; goto done; }
    *                    { kind = ERR; goto done; }
    */
done:
    *pcur = YYCURSOR;
    return kind;
}

// Case-insensitive keywords: the table folds 'A'..'Z' to lower case. `null` is case-sensitive
// and stays in the DFA, since a folded table would also accept `NULL`.
static int lex_sql(const char** pcur) {
    const char* YYCURSOR = *pcur;
    const char* YYMARKER;
    const char* tok = YYCURSOR;
    int kind;
    (void)YYMARKER;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = char;
    re2c:keywords = 1;
    re2c:keywords:model = "archtime";
    re2c:keywords:token = tok;

    "\x00"                   { kind = END; goto done; }
    [ ]+                     { kind = SPACE; goto done; }
    [0-9]+                   { kind = NUM; goto done; }
    'select'                 { kind = KW_SELECT; goto done; }
    'from'                   { kind = KW_FROM; goto done; }
    'where'                  { kind = KW_WHERE; goto done; }
    'order_by'               { kind = KW_ORDER_BY; goto done; }
    "null"                   { kind = KW_NULL; goto done; }
    [a-zA-Z_][a-zA-Z0-9_]*   { kind = ID; goto done; }
    *                        { kind = ERR; goto done; }
    */
done:
    *pcur = YYCURSOR;
    return kind;
}

typedef int (*lexer_t)(const char**);

static void check(lexer_t lex, const char* str, const int* kinds) {
    const char* cur = str;
    for (size_t j = 0;; ++j) {
        int kind = lex(&cur);
        assert(kind == kinds[j]);
        if (kind == END) break;
    }
}

int main(void) {
    static const int c1[] = {KW_IF, SPACE, KW_ELSE, SPACE, KW_WHILE, SPACE, KW_RETURN, SPACE, ID,
        SPACE, ID, SPACE, ID, END};
    static const int c2[] = {KW_CONTINUE_A, SPACE, KW_CONTINUE_B, SPACE, ID, SPACE,
        KW_THREAD_LOCAL_VAR, SPACE, ID, SPACE, ID, END};
    static const int c3[] = {KW_PLUS, SPACE, NUM, SPACE, ERR, SPACE, ERR, ERR, END};
    static const int s1[] = {KW_SELECT, SPACE, KW_FROM, SPACE, KW_WHERE, SPACE, KW_SELECT, SPACE,
        KW_FROM, SPACE, KW_WHERE, SPACE, ID, SPACE, ID, END};
    static const int s2[] = {KW_ORDER_BY, SPACE, KW_ORDER_BY, SPACE, KW_NULL, SPACE, ID, SPACE, ID,
        SPACE, NUM, END};
    // Sanity check of the table layout shared with c2simd.
    assert(sizeof(c2_kw_table_t) == 5464);
    check(lex_c, "if else while return iff whil returns", c1);
    check(lex_c, "continue_a continue_b continue_c thread_local_var thread_local_va thread_local_vars", c2);
    check(lex_c, "++ 42 # IF", c3);
    check(lex_sql, "select from where SELECT From wHeRe selects fro", s1);
    check(lex_sql, "order_by ORDER_BY null NULL Null 7", s2);
    return 0;
}
