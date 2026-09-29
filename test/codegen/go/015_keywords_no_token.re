//go:generate re2go $INPUT -o $OUTPUT -i --api simple
package main

// Error: `re2c:keywords` needs `re2c:keywords:token`.
func lex(yyinput []byte) int {
	var yycursor int
	/*!re2c
	re2c:yyfill:enable = 0;
	re2c:YYCTYPE = byte;
	re2c:keywords = 1;

	"if"     { return 1 }
	[a-z]+   { return 2 }
	*        { return 3 }
	*/
}
