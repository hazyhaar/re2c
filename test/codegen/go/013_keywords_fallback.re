//go:generate re2go $INPUT -o $OUTPUT -i --api simple
package main

/*!keywords:re2c*/

// No literal rule is subsumed by a later rule: the keyword table is not used, the lexer is the
// plain DFA and re2c warns about it.
func lex(yyinput []byte) int {
	var yycursor int
	/*!re2c
	re2c:yyfill:enable = 0;
	re2c:YYCTYPE = byte;
	re2c:keywords = 1;
	re2c:keywords:token = 0;

	"if"     { return 1 }
	[0-9]+   { return 2 }
	*        { return 3 }
	*/
}

func main() {
	if lex([]byte("if\x00")) != 1 || lex([]byte("12\x00")) != 2 || lex([]byte("i\x00")) != 3 {
		panic("error")
	}
}
