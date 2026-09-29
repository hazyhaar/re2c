//go:generate re2go $INPUT -o $OUTPUT -i --api simple
package main

// Error: the keyword table is used in the lexer, but it is not emitted anywhere.
func lex(yyinput []byte) int {
	var yycursor int
	tok := yycursor
	/*!re2c
	re2c:yyfill:enable = 0;
	re2c:YYCTYPE = byte;
	re2c:keywords = 1;
	re2c:keywords:token = tok;

	"if"     { return 1 }
	[a-z]+   { return 2 }
	*        { return 3 }
	*/
}
