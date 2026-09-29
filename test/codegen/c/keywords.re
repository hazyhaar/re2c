// re2c $INPUT -o $OUTPUT -ci
#include <assert.h>
#include <string.h>

// Keyword tables for keyword rules subsumed by the identifier rule.
/*!keywords:re2c*/

/*!conditions:re2c*/

enum { END, ID, NUM, SPACE, ERR, KW_IF, KW_ELSE, KW_WHILE, KW_BEGIN, KW_END, KW_RETURN,
    KW_CONTINUE, KW_EXCLUSIVE, KW_AUTO_INCREMENT, KW_CURRENT_TIMESTAMP, STR };

// Lex one token starting at `*pcur` in condition `*pcond`; keyword `begin` enters condition
// `body`, where keywords `end` (back to `init`) and `return` are recognized.
static int lex(const char** pcur, int* pcond) {
    const char* YYCURSOR = *pcur;
    const char* YYMARKER;
    const char* tok = YYCURSOR;
    int kind;
    (void)YYMARKER;
#define YYGETCONDITION() *pcond
#define YYSETCONDITION(c) *pcond = c
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:define:YYCTYPE = char;
    re2c:keywords = 1;
    re2c:keywords:token = tok;

    <*> "\x00"                    { kind = END; goto done; }
    <*> [ ]+                      { kind = SPACE; goto done; }
    <*> [0-9]+                    { kind = NUM; goto done; }
    <init> "if"                   { kind = KW_IF; goto done; }
    <init> "else"                 { kind = KW_ELSE; goto done; }
    <init> "while"                { kind = KW_WHILE; goto done; }
    <init> "begin" => body        { kind = KW_BEGIN; goto done; }
    <init> "exclusive"            { kind = KW_EXCLUSIVE; goto done; }
    <init> "auto_increment"       { kind = KW_AUTO_INCREMENT; goto done; }
    <init> "current_timestamp"    { kind = KW_CURRENT_TIMESTAMP; goto done; }
    <body> "end" => init          { kind = KW_END; goto done; }
    <body> "return"               { kind = KW_RETURN; goto done; }
    <body> "continue"             { kind = KW_CONTINUE; goto done; }
    <*> [a-z_][a-z0-9_]*          { kind = ID; goto done; }
    <body> "'" [^'\x00]* "'"      { kind = STR; goto done; }
    <*> *                         { kind = ERR; goto done; }
    */
done:
    *pcur = YYCURSOR;
    return kind;
#undef YYGETCONDITION
#undef YYSETCONDITION
}

int main(void) {
    static const struct { const char* str; int kinds[16]; } tests[] = {
        {"if else while iff els whilex", {KW_IF, SPACE, KW_ELSE, SPACE, KW_WHILE, SPACE, ID,
            SPACE, ID, SPACE, ID, END}},
        {"end return begin end return end", {ID, SPACE, ID, SPACE, KW_BEGIN, SPACE, KW_END,
            SPACE, ID, SPACE, ID, END}},
        {"begin if return 'x' continue end if", {KW_BEGIN, SPACE, ID, SPACE, KW_RETURN, SPACE, STR,
            SPACE, KW_CONTINUE, SPACE, KW_END, SPACE, KW_IF, END}},
        {"exclusive exclusives auto_increment auto_incremen", {KW_EXCLUSIVE, SPACE, ID, SPACE,
            KW_AUTO_INCREMENT, SPACE, ID, END}},
        {"current_timestamp current_timestam 12 -", {KW_CURRENT_TIMESTAMP, SPACE, ID, SPACE,
            NUM, SPACE, ERR, END}},
    };
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        const char* cur = tests[i].str;
        int cond = yycinit;
        for (size_t j = 0;; ++j) {
            int kind = lex(&cur, &cond);
            assert(kind == tests[i].kinds[j]);
            if (kind == END) break;
        }
    }
    return 0;
}
