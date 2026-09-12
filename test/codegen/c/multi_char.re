// re2c $INPUT -o $OUTPUT -i --multi-char
#include <assert.h>
#include <stdint.h>
#include <string.h>

#define YYCTYPE char
#define YYPEEKN(s, n) ({ \
    uint64_t v = 0; \
    memcpy(&v, s, (n)); \
    v; \
})
#define YYSKIPN(s, n) (s += (n))

static int lex(const char *YYCURSOR)
{
    const char *YYMARKER;
    /*!re2c
    re2c:yyfill:enable = 0;

    *             { return -1; }
    "SELECT"      { return 1; }
    "INSERT"      { return 2; }
    "UPDATE"      { return 3; }
    "DELETE"      { return 4; }
    */
}

int main()
{
    assert(lex("SELECT") == 1);
    assert(lex("INSERT") == 2);
    assert(lex("UPDATE") == 3);
    assert(lex("DELETE") == 4);
    assert(lex("UNKNOWN") == -1);
    return 0;
}
