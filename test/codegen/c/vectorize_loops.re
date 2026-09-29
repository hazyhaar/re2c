// re2c $INPUT -o $OUTPUT -i --vectorize-loops
#include <assert.h>
#include <immintrin.h>
#include <string.h>

static int fill(int need) { (void)need; return 0; }

static int lex_vector(const unsigned char *input, size_t limit, const unsigned char **out_cur)
{
    const unsigned char *YYCURSOR = input;
    const unsigned char *YYLIMIT = input + limit;
    const unsigned char *YYMARKER = input;
    (void)YYMARKER;
    /*!re2c
    re2c:vectorize:loops = 1;
    re2c:define:YYCTYPE = "unsigned char";
    re2c:YYFILL = "fill(0);";
    re2c:YYFILL:naked = 1;

    [0-9]+                 { *out_cur = YYCURSOR; return 1; }
    [a-zA-Z_][a-zA-Z0-9_]* { *out_cur = YYCURSOR; return 2; }
    [ \t]+                 { *out_cur = YYCURSOR; return 3; }
    "\x00"                 { *out_cur = YYCURSOR; return 0; }
    *                      { *out_cur = YYCURSOR; return -1; }
    */
}

static int lex_scalar(const unsigned char *input, size_t limit, const unsigned char **out_cur)
{
    const unsigned char *YYCURSOR = input;
    const unsigned char *YYLIMIT = input + limit;
    const unsigned char *YYMARKER = input;
    (void)YYMARKER;
    /*!re2c
    re2c:vectorize:loops = 0;
    re2c:define:YYCTYPE = "unsigned char";
    re2c:YYFILL = "fill(0);";
    re2c:YYFILL:naked = 1;

    [0-9]+                 { *out_cur = YYCURSOR; return 1; }
    [a-zA-Z_][a-zA-Z0-9_]* { *out_cur = YYCURSOR; return 2; }
    [ \t]+                 { *out_cur = YYCURSOR; return 3; }
    "\x00"                 { *out_cur = YYCURSOR; return 0; }
    *                      { *out_cur = YYCURSOR; return -1; }
    */
}

int main(void)
{
    unsigned char buf[256];
    const unsigned char *cur_v = NULL;
    const unsigned char *cur_s = NULL;

    memset(buf, 'A', sizeof(buf));
    for (size_t n = 1; n < 120; ++n) {
        buf[n] = 0;
        int res_v = lex_vector(buf, n + 1, &cur_v);
        int res_s = lex_scalar(buf, n + 1, &cur_s);
        assert(res_v == res_s);
        assert(res_v == 2);
        assert(cur_v == cur_s);
        buf[n] = 'A';
    }

    memset(buf, '5', sizeof(buf));
    for (size_t n = 1; n < 120; ++n) {
        buf[n] = 0;
        int res_v = lex_vector(buf, n + 1, &cur_v);
        int res_s = lex_scalar(buf, n + 1, &cur_s);
        assert(res_v == res_s);
        assert(res_v == 1);
        assert(cur_v == cur_s);
        buf[n] = '5';
    }

    memset(buf, ' ', sizeof(buf));
    for (size_t n = 1; n < 120; ++n) {
        buf[n] = 0;
        int res_v = lex_vector(buf, n + 1, &cur_v);
        int res_s = lex_scalar(buf, n + 1, &cur_s);
        assert(res_v == res_s);
        assert(res_v == 3);
        assert(cur_v == cur_s);
        buf[n] = ' ';
    }

    return 0;
}
