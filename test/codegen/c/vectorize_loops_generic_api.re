// re2c $INPUT -o $OUTPUT -i --api generic --vectorize-loops
#include <assert.h>
#include <immintrin.h>
#include <stddef.h>
#include <string.h>

static int lex_functions(const char *str, size_t len) {
    const char *cur = str;
    const char *lim = str + len;
    const char *mar = str;
    (void)mar;

#define YYPEEK()       ((unsigned char)*cur)
#define YYSKIP()       (++cur)
#define YYBACKUP()     (mar = cur)
#define YYRESTORE()    (cur = mar)
#define YYSHIFT(n)     (cur += (n))
#define YYLESSTHAN(n)  ((size_t)(lim - cur) < (size_t)(n))
#define YYFILL(n)      return -2

    /*!re2c
        re2c:api = generic;
        re2c:api:style = functions;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = cur;
        re2c:YYFILL:naked = 0;

        [a-zA-Z_][a-zA-Z0-9_]* { return (int)(cur - str); }
        *                      { return -1; }
    */

#undef YYPEEK
#undef YYSKIP
#undef YYBACKUP
#undef YYRESTORE
#undef YYSHIFT
#undef YYLESSTHAN
#undef YYFILL
}

static int lex_freeform(const char *str, size_t len) {
    const char *cur = str;
    const char *lim = str + len;
    const char *mar = str;
    (void)mar;

    /*!re2c
        re2c:api = generic;
        re2c:api:style = free-form;
        re2c:api:sigil = "@@";
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = cur;
        re2c:YYPEEK = "(unsigned char)*cur";
        re2c:YYSKIP = "++cur;";
        re2c:YYBACKUP = "mar = cur;";
        re2c:YYRESTORE = "cur = mar;";
        re2c:YYSHIFT = "cur += @@;";
        re2c:YYLESSTHAN = "(size_t)(lim - cur) < (size_t)(@@)";
        re2c:YYFILL = "return -2;";
        re2c:YYFILL:naked = 1;

        [a-zA-Z_][a-zA-Z0-9_]* { return (int)(cur - str); }
        *                      { return -1; }
    */
}

int main(void) {
    char buf[256];
    memset(buf, 'a', 100);
    buf[100] = '!';
    memset(buf + 101, 0, sizeof(buf) - 101);

    assert(lex_functions(buf, sizeof(buf)) == 100);
    assert(lex_freeform(buf, sizeof(buf)) == 100);
    return 0;
}
