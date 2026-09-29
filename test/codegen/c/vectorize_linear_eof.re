// re2c $INPUT -o $OUTPUT -i
// Parity of --vectorize-linear with the ordinary DFA with the end-of-input rule and YYFILL: a
// stream of tokens is lexed through a small buffer refilled from memory, for many buffer sizes.
// The buffer is allocated with its exact size (BUFSIZE + 1 for the sentinel); an out-of-bounds read
// is detected by AddressSanitizer.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const unsigned char* src;
    size_t srclen, srcpos, bufsize;
    unsigned char *buf, *lim, *cur, *tok, *mar;
    int eof;
} Input;

static int fill(Input* in) {
    if (in->eof) return 1;
    const size_t shift = (size_t)(in->tok - in->buf);
    const size_t used = (size_t)(in->lim - in->tok);
    if (shift < 1) return 2;
    memmove(in->buf, in->tok, used);
    in->lim -= shift;
    in->cur -= shift;
    in->tok -= shift;
    in->mar -= shift;
    size_t n = in->bufsize - used;
    if (n > in->srclen - in->srcpos) n = in->srclen - in->srcpos;
    memcpy(in->lim, in->src + in->srcpos, n);
    in->srcpos += n;
    in->lim += n;
    *in->lim = 0;
    if (in->srcpos == in->srclen) in->eof = 1;
    return 0;
}

// Returns the rule number, 100 at the end of input, -1 on error; `*len` is the token length.
typedef int (*lex_t)(Input*, size_t*);
struct word { const char* str; size_t len; };

/*!rules:re2c:headers
    "Content-Length:"            { *len = (size_t)(in->cur - in->tok); return 1; }
    "Content-Type:"              { *len = (size_t)(in->cur - in->tok); return 2; }
    "Transfer-Encoding:"         { *len = (size_t)(in->cur - in->tok); return 3; }
    "Authorization:"             { *len = (size_t)(in->cur - in->tok); return 4; }
    "Accept-Encoding:"           { *len = (size_t)(in->cur - in->tok); return 5; }
    "User-Agent:"                { *len = (size_t)(in->cur - in->tok); return 6; }
    "Cache-Control:"             { *len = (size_t)(in->cur - in->tok); return 7; }
    "Connection:"                { *len = (size_t)(in->cur - in->tok); return 8; }
    [a-zA-Z-]+ ":"               { *len = (size_t)(in->cur - in->tok); return 9; }
    *                            { *len = (size_t)(in->cur - in->tok); return 0; }
*/

static int headers_0(Input* in, size_t* len) {
    in->tok = in->cur;
    /*!use:re2c:headers
        re2c:api:style = free-form;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = in->cur;
        re2c:YYMARKER = in->mar;
        re2c:YYLIMIT = in->lim;
        re2c:YYFILL = "fill(in) == 0";
        re2c:eof = 0;
        re2c:vectorize:linear = 0;

        $ { *len = 0; return 100; }
    */
}

static int headers_1(Input* in, size_t* len) {
    in->tok = in->cur;
    /*!use:re2c:headers
        re2c:api:style = free-form;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = in->cur;
        re2c:YYMARKER = in->mar;
        re2c:YYLIMIT = in->lim;
        re2c:YYFILL = "fill(in) == 0";
        re2c:eof = 0;
        re2c:vectorize:linear = 1;

        $ { *len = 0; return 100; }
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
    "SELECT"                     { *len = (size_t)(in->cur - in->tok); return 1; }
    "INSERT"                     { *len = (size_t)(in->cur - in->tok); return 2; }
    "UPDATE"                     { *len = (size_t)(in->cur - in->tok); return 3; }
    "DELETE"                     { *len = (size_t)(in->cur - in->tok); return 4; }
    "FROM"                       { *len = (size_t)(in->cur - in->tok); return 5; }
    "WHERE"                      { *len = (size_t)(in->cur - in->tok); return 6; }
    "BETWEEN"                    { *len = (size_t)(in->cur - in->tok); return 7; }
    "DISTINCT"                   { *len = (size_t)(in->cur - in->tok); return 8; }
    [a-zA-Z_][a-zA-Z0-9_]*       { *len = (size_t)(in->cur - in->tok); return 9; }
    [ ]+                         { *len = (size_t)(in->cur - in->tok); return 10; }
    *                            { *len = (size_t)(in->cur - in->tok); return 0; }
*/

static int keywords_0(Input* in, size_t* len) {
    in->tok = in->cur;
    /*!use:re2c:keywords
        re2c:api:style = free-form;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = in->cur;
        re2c:YYMARKER = in->mar;
        re2c:YYLIMIT = in->lim;
        re2c:YYFILL = "fill(in) == 0";
        re2c:eof = 0;
        re2c:vectorize:linear = 0;

        $ { *len = 0; return 100; }
    */
}

static int keywords_1(Input* in, size_t* len) {
    in->tok = in->cur;
    /*!use:re2c:keywords
        re2c:api:style = free-form;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = in->cur;
        re2c:YYMARKER = in->mar;
        re2c:YYLIMIT = in->lim;
        re2c:YYFILL = "fill(in) == 0";
        re2c:eof = 0;
        re2c:vectorize:linear = 1;

        $ { *len = 0; return 100; }
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
    "ab"                         { *len = (size_t)(in->cur - in->tok); return 1; }
    "abcd"                       { *len = (size_t)(in->cur - in->tok); return 2; }
    "abcdefgh"                   { *len = (size_t)(in->cur - in->tok); return 3; }
    "abcdefghijk"                { *len = (size_t)(in->cur - in->tok); return 4; }
    "abcx"                       { *len = (size_t)(in->cur - in->tok); return 5; }
    "bcdefghi"                   { *len = (size_t)(in->cur - in->tok); return 6; }
    "xyzzyxyzzy"                 { *len = (size_t)(in->cur - in->tok); return 7; }
    "q" [0-9]+ "end"             { *len = (size_t)(in->cur - in->tok); return 8; }
    *                            { *len = (size_t)(in->cur - in->tok); return 0; }
*/

static int backtrack_0(Input* in, size_t* len) {
    in->tok = in->cur;
    /*!use:re2c:backtrack
        re2c:api:style = free-form;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = in->cur;
        re2c:YYMARKER = in->mar;
        re2c:YYLIMIT = in->lim;
        re2c:YYFILL = "fill(in) == 0";
        re2c:eof = 0;
        re2c:vectorize:linear = 0;

        $ { *len = 0; return 100; }
    */
}

static int backtrack_1(Input* in, size_t* len) {
    in->tok = in->cur;
    /*!use:re2c:backtrack
        re2c:api:style = free-form;
        re2c:YYCTYPE = "unsigned char";
        re2c:YYCURSOR = in->cur;
        re2c:YYMARKER = in->mar;
        re2c:YYLIMIT = in->lim;
        re2c:YYFILL = "fill(in) == 0";
        re2c:eof = 0;
        re2c:vectorize:linear = 1;

        $ { *len = 0; return 100; }
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

static int failures = 0;

static void run(lex_t f, Input* in, const unsigned char* src, size_t len, size_t bufsize,
        int* rules, size_t* lens, size_t* ntok) {
    in->src = src;
    in->srclen = len;
    in->srcpos = 0;
    in->bufsize = bufsize;
    in->buf = (unsigned char*)malloc(bufsize + 1);
    in->cur = in->tok = in->mar = in->lim = in->buf + bufsize;
    *in->lim = 0;
    in->eof = 0;
    *ntok = 0;
    for (;;) {
        size_t l = 0;
        const int r = f(in, &l);
        rules[*ntok] = r;
        lens[*ntok] = l;
        ++*ntok;
        if (r == 100 || r == -1) break;
    }
    free(in->buf);
}

static void check_set(lex_t f, lex_t g, const struct word* ws, const char* name) {
    // Build a stream of words, their prefixes and near misses, separated by a few characters.
    static const char seps[] = " :-\t";
    unsigned char* src = (unsigned char*)malloc(1 << 16);
    size_t len = 0, k = 0;
    for (const struct word* w = ws; w->str; ++w) {
        for (size_t i = 1; i <= w->len; ++i) {
            memcpy(src + len, w->str, i);
            len += i;
            src[len++] = (unsigned char)seps[k++ % (sizeof(seps) - 1)];
            memcpy(src + len, w->str, i);
            len += i;
            if (i < w->len) src[len++] = (unsigned char)(w->str[i] + 1);
            src[len++] = (unsigned char)seps[k++ % (sizeof(seps) - 1)];
        }
        for (const struct word* v = ws; v->str; ++v) {
            memcpy(src + len, w->str, w->len);
            len += w->len;
            memcpy(src + len, v->str, v->len);
            len += v->len;
            src[len++] = (unsigned char)seps[k++ % (sizeof(seps) - 1)];
        }
    }
    int* r1 = (int*)malloc(sizeof(int) * (len + 2));
    int* r2 = (int*)malloc(sizeof(int) * (len + 2));
    size_t* l1 = (size_t*)malloc(sizeof(size_t) * (len + 2));
    size_t* l2 = (size_t*)malloc(sizeof(size_t) * (len + 2));
    for (size_t bufsize = 24; bufsize <= 56; ++bufsize) {
        Input in;
        size_t n1, n2;
        run(f, &in, src, len, bufsize, r1, l1, &n1);
        run(g, &in, src, len, bufsize, r2, l2, &n2);
        if (n1 != n2 || r1[n1 - 1] != 100) {
            fprintf(stderr, "%s, buffer %d: %d/%d tokens, last rule %d\n",
                name, (int)bufsize, (int)n1, (int)n2, r1[n1 - 1]);
            ++failures;
            continue;
        }
        for (size_t i = 0; i < n1; ++i) {
            if (r1[i] != r2[i] || l1[i] != l2[i]) {
                fprintf(stderr, "%s, buffer %d, token %d: rule %d/%d, length %d/%d\n",
                    name, (int)bufsize, (int)i, r1[i], r2[i], (int)l1[i], (int)l2[i]);
                ++failures;
                break;
            }
        }
    }
    free(r1); free(r2); free(l1); free(l2); free(src);
}


int main() {
    check_set(headers_0, headers_1, headers_words, "headers");
    check_set(keywords_0, keywords_1, keywords_words, "keywords");
    check_set(backtrack_0, backtrack_1, backtrack_words, "backtrack");
    return failures > 0 ? 1 : 0;
}
