// re2c $INPUT -o $OUTPUT -i --eager-skip
// Parity of --vectorize-linear with the ordinary DFA, sentinel method without YYFILL.
// Each rule set is compiled twice, with and without the option; both lexers must return the same
// rule and the same cursor for every prefix and near miss of the literals, with the limit at the
// sentinel, after it, and before trailing padding. Buffers are allocated with their exact size, so
// that an out-of-bounds read is detected by AddressSanitizer.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*lex_t)(const unsigned char*, const unsigned char*, const unsigned char**);
struct word { const char* str; size_t len; };

#define RET(n) do { *end = YYCURSOR; return n; } while (0)

/*!rules:re2c:headers
    "Content-Length:"            { RET(1); }
    "Content-Type:"              { RET(2); }
    "Transfer-Encoding:"         { RET(3); }
    "Authorization:"             { RET(4); }
    "Accept-Encoding:"           { RET(5); }
    "User-Agent:"                { RET(6); }
    "Cache-Control:"             { RET(7); }
    "Connection:"                { RET(8); }
    [a-zA-Z-]+ ":"               { RET(9); }
    *                            { RET(0); }
*/

static int headers_0(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:headers
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 0;
        re2c:YYCTYPE = "unsigned char";
    */
}

static int headers_1(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:headers
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 1;
        re2c:YYCTYPE = "unsigned char";
    */
}

static const struct word headers_words[] = {
    {"\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x4c\x65\x6e\x67\x74\x68\x3a", 15},
    {"\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x54\x79\x70\x65\x3a", 13},
    {"\x54\x72\x61\x6e\x73\x66\x65\x72\x2d\x45\x6e\x63\x6f\x64\x69\x6e\x67\x3a", 18},
    {"\x41\x75\x74\x68\x6f\x72\x69\x7a\x61\x74\x69\x6f\x6e\x3a", 14},
    {"\x41\x63\x63\x65\x70\x74\x2d\x45\x6e\x63\x6f\x64\x69\x6e\x67\x3a", 16},
    {"\x55\x73\x65\x72\x2d\x41\x67\x65\x6e\x74\x3a", 11},
    {"\x43\x61\x63\x68\x65\x2d\x43\x6f\x6e\x74\x72\x6f\x6c\x3a", 14},
    {"\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x3a", 11},
    {"\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x4c\x61\x6e\x67\x75\x61\x67\x65\x3a", 17},
    {"\x41\x63\x63\x65\x70\x74\x3a", 7},
    {"\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x2d\x49\x64\x3a", 14},
    {"\x43\x61\x63\x68\x65\x2d\x53\x74\x61\x74\x75\x73\x3a", 13},
    {NULL, 0}
};

/*!rules:re2c:keywords
    "SELECT"                     { RET(1); }
    "INSERT"                     { RET(2); }
    "UPDATE"                     { RET(3); }
    "DELETE"                     { RET(4); }
    "FROM"                       { RET(5); }
    "WHERE"                      { RET(6); }
    "BETWEEN"                    { RET(7); }
    "DISTINCT"                   { RET(8); }
    [a-zA-Z_][a-zA-Z0-9_]*       { RET(9); }
    [ ]+                         { RET(10); }
    *                            { RET(0); }
*/

static int keywords_0(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:keywords
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 0;
        re2c:YYCTYPE = "unsigned char";
    */
}

static int keywords_1(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:keywords
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 1;
        re2c:YYCTYPE = "unsigned char";
    */
}

static const struct word keywords_words[] = {
    {"\x53\x45\x4c\x45\x43\x54", 6},
    {"\x49\x4e\x53\x45\x52\x54", 6},
    {"\x55\x50\x44\x41\x54\x45", 6},
    {"\x44\x45\x4c\x45\x54\x45", 6},
    {"\x46\x52\x4f\x4d", 4},
    {"\x57\x48\x45\x52\x45", 5},
    {"\x42\x45\x54\x57\x45\x45\x4e", 7},
    {"\x44\x49\x53\x54\x49\x4e\x43\x54", 8},
    {"\x53\x45\x4c\x45\x43\x54\x45\x44", 8},
    {"\x44\x49\x53\x54\x49\x4e\x43\x54\x49\x4f\x4e", 11},
    {"\x46\x52\x4f\x4d\x41\x47\x45", 7},
    {"\x57\x48\x45\x52\x45\x41\x53", 7},
    {NULL, 0}
};

/*!rules:re2c:backtrack
    "ab"                         { RET(1); }
    "abcd"                       { RET(2); }
    "abcdefgh"                   { RET(3); }
    "abcdefghijk"                { RET(4); }
    "abcx"                       { RET(5); }
    "bcdefghi"                   { RET(6); }
    "xyzzyxyzzy"                 { RET(7); }
    "q" [0-9]+ "end"             { RET(8); }
    *                            { RET(0); }
*/

static int backtrack_0(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:backtrack
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 0;
        re2c:YYCTYPE = "unsigned char";
    */
}

static int backtrack_1(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:backtrack
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 1;
        re2c:YYCTYPE = "unsigned char";
    */
}

static const struct word backtrack_words[] = {
    {"\x61\x62", 2},
    {"\x61\x62\x63\x64", 4},
    {"\x61\x62\x63\x64\x65\x66\x67\x68", 8},
    {"\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b", 11},
    {"\x61\x62\x63\x78", 4},
    {"\x62\x63\x64\x65\x66\x67\x68\x69", 8},
    {"\x78\x79\x7a\x7a\x79\x78\x79\x7a\x7a\x79", 10},
    {"\x71\x31\x32\x33\x65\x6e\x64", 7},
    {"\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a", 10},
    {"\x78\x79\x7a\x7a\x79\x78\x79\x7a\x7a", 9},
    {NULL, 0}
};

/*!rules:re2c:binary
    "\x89PNG\x0d\x0a\x1a\x0a"    { RET(1); }
    "GIF89a"                     { RET(2); }
    "\xef\xbb\xbf"               { RET(3); }
    "PK\x03\x04"                 { RET(4); }
    "%PDF-1."                    { RET(5); }
    "\xff\xd8\xff\xe0"           { RET(6); }
    *                            { RET(0); }
*/

static int binary_0(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:binary
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 0;
        re2c:YYCTYPE = "unsigned char";
    */
}

static int binary_1(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:binary
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 1;
        re2c:YYCTYPE = "unsigned char";
    */
}

static const struct word binary_words[] = {
    {"\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8},
    {"\x47\x49\x46\x38\x39\x61", 6},
    {"\x47\x49\x46\x38\x37\x61", 6},
    {"\xef\xbb\xbf", 3},
    {"\x50\x4b\x03\x04", 4},
    {"\x25\x50\x44\x46\x2d\x31\x2e", 7},
    {"\xff\xd8\xff\xe0", 4},
    {"\xff\xd8\xff\xe1", 4},
    {NULL, 0}
};

/*!rules:re2c:anchored
    "GIF89a"                     { RET(1); }
    "GIF87a"                     { RET(2); }
    [\x00\x01]                   { RET(3); }
    [^\x00\x01] [\x00\x01]       { RET(3); }
    [^\x00\x01] [^\x00\x01]      { RET(0); }
*/

static int anchored_0(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:anchored
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 0;
        re2c:YYCTYPE = "unsigned char";
    */
}

static int anchored_1(
        const unsigned char* YYCURSOR, const unsigned char* YYLIMIT, const unsigned char** end) {
    const unsigned char* YYMARKER;
    (void)YYLIMIT;
    /*!use:re2c:anchored
        re2c:yyfill:enable = 0;
        re2c:vectorize:linear = 1;
        re2c:YYCTYPE = "unsigned char";
    */
}

static const struct word anchored_words[] = {
    {"\x47\x49\x46\x38\x39\x61", 6},
    {"\x47\x49\x46\x38\x37\x61", 6},
    {"\x47\x49\x46\x38", 4},
    {"\x47\x49", 2},
    {"\x47", 1},
    {"\x47\x4a", 2},
    {NULL, 0}
};

static int failures = 0;
static long checks = 0;

static void check1(lex_t f, lex_t g, const unsigned char* in, size_t len, size_t pad, size_t lim) {
    unsigned char* buf = (unsigned char*)malloc(len + 1 + pad);
    if (len > 0) memcpy(buf, in, len);
    buf[len] = 0;
    for (size_t i = 0; i < pad; ++i) buf[len + 1 + i] = len > 0 ? in[i % len] : 'a';
    const unsigned char *e1 = NULL, *e2 = NULL;
    const int r1 = f(buf, buf + lim, &e1);
    const int r2 = g(buf, buf + lim, &e2);
    ++checks;
    if (r1 != r2 || e1 != e2) {
        fprintf(stderr, "mismatch on input of length %d: rule %d/%d, length %d/%d\n",
            (int)len, r1, r2, (int)(e1 - buf), (int)(e2 - buf));
        ++failures;
    }
    free(buf);
}

static void check(lex_t f, lex_t g, const unsigned char* in, size_t len) {
    check1(f, g, in, len, 0, len);           // limit at the sentinel
    check1(f, g, in, len, 0, len + 1);       // limit after the sentinel
    check1(f, g, in, len, 16, len + 1 + 16); // limit after padding
}

static void check_set(lex_t f, lex_t g, const struct word* ws) {
    static const unsigned char alt[] = {0x00, 0x01, 'a', 'z', 'A', 'Z', ':', '-', ' ', '_', '0',
        0x7f, 0x80, 0xfe, 0xff};
    unsigned char t[128];
    for (const struct word* w = ws; w->str; ++w) {
        const unsigned char* s = (const unsigned char*)w->str;
        for (size_t i = 0; i <= w->len; ++i) {
            memcpy(t, s, i);
            check(f, g, t, i);
            for (size_t k = 0; k < sizeof(alt); ++k) {
                t[i] = alt[k];
                check(f, g, t, i + 1);
            }
            if (i < w->len) {
                t[i] = (unsigned char)(s[i] + 1);
                check(f, g, t, i + 1);
                t[i] = (unsigned char)(s[i] - 1);
                check(f, g, t, i + 1);
            }
        }
        for (const struct word* v = ws; v->str; ++v) {
            memcpy(t, s, w->len);
            memcpy(t + w->len, v->str, v->len);
            check(f, g, t, w->len + v->len);
        }
    }
}

int main() {
    check_set(headers_0, headers_1, headers_words);
    check_set(keywords_0, keywords_1, keywords_words);
    check_set(backtrack_0, backtrack_1, backtrack_words);
    check_set(binary_0, binary_1, binary_words);
    check_set(anchored_0, anchored_1, anchored_words);
    if (failures > 0) {
        fprintf(stderr, "%d of %ld checks failed\n", failures, checks);
        return 1;
    }
    return 0;
}
