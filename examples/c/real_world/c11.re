// re2c $INPUT -o $OUTPUT -i
//
// C11 language lexer.
// Reference: https://quut.com/c/ANSI-C-grammar-l-2011.html

#include <assert.h>
#include <stdio.h>

constexpr bool DEBUG = false;

#define PRINT_TOKEN(token) do { \
  if (DEBUG) printf("token = [%s]\n", token); \
} while(0)

#define PRINT_TOKEN2(start, size) do { \
  if (DEBUG) printf("token = [%.*s]\n", (int)(size), token); \
} while(0)

bool lex(const char *source) {
  const char *YYCURSOR = source;
  const char *YYMARKER = source;

  while(true){
    const char* token = YYCURSOR;
  /*!re2c
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0;

    O           = [0-7];
    D           = [0-9];
    NZ          = [1-9];
    L           = [a-zA-Z_];
    A           = [a-zA-Z_0-9];
    H           = [a-fA-F0-9];
    HP          = [0][xX];
    E           = [Ee][+-]? D+;
    P           = [Pp][+-]? D+;
    FS          = [fFlL];
    IS          = (([uU])("l"|"L"|"ll"|"LL")?) | (("l"|"L"|"ll"|"LL")([uU])?);
    CP          = "u"|"U"|"L";
    SP          = "u8"|"u"|"U"|"L";
    ES          = "\\" ['"\\abfnrtv] | [0-7]{1,3} | [x][a-fA-F0-9]+;
    WS          = [ \t\v\n\f];
    SPACES      = WS+;
    STR_IT      = (SP? ["] ([^"\\\n\x00] | ES)* ["] WS*)+;
    IDENTIFIER  = L A*;
    ICONST1     = HP H+ IS?;
    ICONST2     = NZ D* IS?;
    ICONST3     = [0] O* IS?;
    ICONST4     = CP? ['] ([^'\\\n\x00]| ES)+ ['];
    FCONST1     = D+ E FS?;
    FCONST2     = D* "." D+ E? FS?;
    FCONST3     = D+ "." E? FS?;
    FCONST4     = HP H+ P FS?;
    FCONST5     = HP H* "." H+ P FS?;
    FCONST6     = HP H+ "." P FS?;
    CMTLine     = "//" [^\n\x00]* "\n";
    CMTStar     = "/*" ([^*\x00] | ("*" [^/\x00]))* "*" "/";
    ST_ASSERT   = "_Static_assert";
    TH_LOCAL    = "_Thread_local";

    // space and comments.
    SPACES       { continue; }
    CMTStar      { continue; }
    CMTLine      { continue; }

    // macros.
    "#"          { PRINT_TOKEN("#"); continue; }
    [\\]         { PRINT_TOKEN("\\"); continue; }

    // keywords.
    "auto"       { PRINT_TOKEN("auto"); continue; }
    "break"      { PRINT_TOKEN("break"); continue; }
    "case"       { PRINT_TOKEN("case"); continue; }
    "char"       { PRINT_TOKEN("char"); continue; }
    "const"      { PRINT_TOKEN("const"); continue; }
    "continue"   { PRINT_TOKEN("continue"); continue; }
    "default"    { PRINT_TOKEN("default"); continue; }
    "do"         { PRINT_TOKEN("do"); continue; }
    "double"     { PRINT_TOKEN("double"); continue; }
    "else"       { PRINT_TOKEN("else"); continue; }
    "enum"       { PRINT_TOKEN("enum"); continue; }
    "extern"     { PRINT_TOKEN("extern"); continue; }
    "float"      { PRINT_TOKEN("float"); continue; }
    "for"        { PRINT_TOKEN("for"); continue; }
    "goto"       { PRINT_TOKEN("goto"); continue; }
    "if"         { PRINT_TOKEN("if"); continue; }
    "inline"     { PRINT_TOKEN("inline"); continue; }
    "int"        { PRINT_TOKEN("int"); continue; }
    "long"       { PRINT_TOKEN("long"); continue; }
    "register"   { PRINT_TOKEN("register"); continue; }
    "restrict"   { PRINT_TOKEN("restrict"); continue; }
    "return"     { PRINT_TOKEN("return"); continue; }
    "short"      { PRINT_TOKEN("short"); continue; }
    "signed"     { PRINT_TOKEN("signed"); continue; }
    "sizeof"     { PRINT_TOKEN("sizeof"); continue; }
    "static"     { PRINT_TOKEN("static"); continue; }
    "struct"     { PRINT_TOKEN("struct"); continue; }
    "switch"     { PRINT_TOKEN("switch"); continue; }
    "typedef"    { PRINT_TOKEN("typedef"); continue; }
    "union"      { PRINT_TOKEN("union"); continue; }
    "unsigned"   { PRINT_TOKEN("unsigned"); continue; }
    "void"       { PRINT_TOKEN("void"); continue; }
    "volatile"   { PRINT_TOKEN("volatile"); continue; }
    "while"      { PRINT_TOKEN("while"); continue; }
    "_Alignas"   { PRINT_TOKEN("_Alignas"); continue; }
    "_Alignof"   { PRINT_TOKEN("_Alignof"); continue; }
    "_Atomic"    { PRINT_TOKEN("_Atomic"); continue; }
    "_Bool"      { PRINT_TOKEN("_Bool"); continue; }
    "_Complex"   { PRINT_TOKEN("_Complex"); continue; }
    "_Generic"   { PRINT_TOKEN("_Generic"); continue; }
    "_Imaginary" { PRINT_TOKEN("_Imaginary"); continue; }
    "_Noreturn"  { PRINT_TOKEN("_Noreturn"); continue; }
    ST_ASSERT    { PRINT_TOKEN("_Static_assert"); continue; }
    TH_LOCAL     { PRINT_TOKEN("_Thread_local"); continue; }
    "__func__"   { PRINT_TOKEN("__func__"); continue; }

    IDENTIFIER   { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ICONST1      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ICONST2      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ICONST3      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ICONST4      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    FCONST1      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    FCONST2      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    FCONST3      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    FCONST4      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    FCONST5      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    FCONST6      { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    STR_IT       { PRINT_TOKEN2(token, YYCURSOR - token); continue; }

    "..."        { PRINT_TOKEN("..."); continue; }
    ">>="        { PRINT_TOKEN(">>="); continue; }
    "<<="        { PRINT_TOKEN("<<="); continue; }
    "+="         { PRINT_TOKEN("+="); continue; }
    "-="         { PRINT_TOKEN("-="); continue; }
    "*="         { PRINT_TOKEN("*="); continue; }
    "/="         { PRINT_TOKEN("/="); continue; }
    "%="         { PRINT_TOKEN("%="); continue; }
    "&="         { PRINT_TOKEN("&="); continue; }
    "^="         { PRINT_TOKEN("^="); continue; }
    "|="         { PRINT_TOKEN("|="); continue; }
    ">>"         { PRINT_TOKEN(">>"); continue; }
    "<<"         { PRINT_TOKEN("<<"); continue; }
    "++"         { PRINT_TOKEN("++"); continue; }
    "--"         { PRINT_TOKEN("--"); continue; }
    "->"         { PRINT_TOKEN("->"); continue; }
    "&&"         { PRINT_TOKEN("&&"); continue; }
    "||"         { PRINT_TOKEN("||"); continue; }
    "<="         { PRINT_TOKEN("<="); continue; }
    ">="         { PRINT_TOKEN(">="); continue; }
    "=="         { PRINT_TOKEN("=="); continue; }
    "!="         { PRINT_TOKEN("!="); continue; }
    ";"          { PRINT_TOKEN(";"); continue; }
    ("{"|"<%")   { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ("}"|"%>")   { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ","          { PRINT_TOKEN(","); continue; }
    ":"          { PRINT_TOKEN(":"); continue; }
    "="          { PRINT_TOKEN("="); continue; }
    "("          { PRINT_TOKEN("("); continue; }
    ")"          { PRINT_TOKEN(")"); continue; }
    ("["|"<:")   { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    ("]"|":>")   { PRINT_TOKEN2(token, YYCURSOR - token); continue; }
    "."          { PRINT_TOKEN("."); continue; }
    "&"          { PRINT_TOKEN("&"); continue; }
    "!"          { PRINT_TOKEN("!"); continue; }
    "~"          { PRINT_TOKEN("~"); continue; }
    "-"          { PRINT_TOKEN("-"); continue; }
    "+"          { PRINT_TOKEN("+"); continue; }
    "*"          { PRINT_TOKEN("*"); continue; }
    "/"          { PRINT_TOKEN("/"); continue; }
    "%"          { PRINT_TOKEN("%"); continue; }
    "<"          { PRINT_TOKEN("<"); continue; }
    ">"          { PRINT_TOKEN(">"); continue; }
    "^"          { PRINT_TOKEN("^"); continue; }
    "|"          { PRINT_TOKEN("|"); continue; }
    "?"          { PRINT_TOKEN("?"); continue; }

    *            { printf("[Error] unknown char = [%d]\n", yych); return false; }
    [\x00]       { return true; }
  */
  }

  return 0;
}

int main() {
  const char* str = R"~(// Test C language(C11) lexer.
    #include <stdio.h> 
    /* hello world example. */
    int main(){ 
      printf("hello world\n");
      return 0; 
    }
  )~";
  assert(lex(str));
  return 0;
}
