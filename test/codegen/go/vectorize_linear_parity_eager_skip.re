//go:generate re2go $INPUT -o $OUTPUT -i --eager-skip
// Parity of --vectorize-linear with the ordinary DFA for the simple API without YYFILL.
// Each rule set is compiled twice, with and without the option; both lexers must return the same
// rule and the same cursor for every prefix and near miss of the literals, with and without
// trailing padding after the sentinel. An out-of-range read panics.
package main

import (
	"fmt"
	"os"
)

/*!rules:re2c:headers
	"Content-Length:"            { return 1, yycursor }
	"Content-Type:"              { return 2, yycursor }
	"Transfer-Encoding:"         { return 3, yycursor }
	"Authorization:"             { return 4, yycursor }
	"Accept-Encoding:"           { return 5, yycursor }
	"User-Agent:"                { return 6, yycursor }
	"Cache-Control:"             { return 7, yycursor }
	"Connection:"                { return 8, yycursor }
	[a-zA-Z-]+ ":"               { return 9, yycursor }
	*                            { return 0, yycursor }
*/

func headers_0(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:headers
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 0;
		re2c:YYCTYPE = byte;
	*/
}

func headers_1(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:headers
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 1;
		re2c:YYCTYPE = byte;
	*/
}

var headers_words = []string{
	"\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x4c\x65\x6e\x67\x74\x68\x3a",
	"\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x54\x79\x70\x65\x3a",
	"\x54\x72\x61\x6e\x73\x66\x65\x72\x2d\x45\x6e\x63\x6f\x64\x69\x6e\x67\x3a",
	"\x41\x75\x74\x68\x6f\x72\x69\x7a\x61\x74\x69\x6f\x6e\x3a",
	"\x41\x63\x63\x65\x70\x74\x2d\x45\x6e\x63\x6f\x64\x69\x6e\x67\x3a",
	"\x55\x73\x65\x72\x2d\x41\x67\x65\x6e\x74\x3a",
	"\x43\x61\x63\x68\x65\x2d\x43\x6f\x6e\x74\x72\x6f\x6c\x3a",
	"\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x3a",
	"\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x4c\x61\x6e\x67\x75\x61\x67\x65\x3a",
	"\x41\x63\x63\x65\x70\x74\x3a",
	"\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x2d\x49\x64\x3a",
	"\x43\x61\x63\x68\x65\x2d\x53\x74\x61\x74\x75\x73\x3a",
}

/*!rules:re2c:keywords
	"SELECT"                     { return 1, yycursor }
	"INSERT"                     { return 2, yycursor }
	"UPDATE"                     { return 3, yycursor }
	"DELETE"                     { return 4, yycursor }
	"FROM"                       { return 5, yycursor }
	"WHERE"                      { return 6, yycursor }
	"BETWEEN"                    { return 7, yycursor }
	"DISTINCT"                   { return 8, yycursor }
	[a-zA-Z_][a-zA-Z0-9_]*       { return 9, yycursor }
	[ ]+                         { return 10, yycursor }
	*                            { return 0, yycursor }
*/

func keywords_0(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:keywords
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 0;
		re2c:YYCTYPE = byte;
	*/
}

func keywords_1(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:keywords
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 1;
		re2c:YYCTYPE = byte;
	*/
}

var keywords_words = []string{
	"\x53\x45\x4c\x45\x43\x54",
	"\x49\x4e\x53\x45\x52\x54",
	"\x55\x50\x44\x41\x54\x45",
	"\x44\x45\x4c\x45\x54\x45",
	"\x46\x52\x4f\x4d",
	"\x57\x48\x45\x52\x45",
	"\x42\x45\x54\x57\x45\x45\x4e",
	"\x44\x49\x53\x54\x49\x4e\x43\x54",
	"\x53\x45\x4c\x45\x43\x54\x45\x44",
	"\x44\x49\x53\x54\x49\x4e\x43\x54\x49\x4f\x4e",
	"\x46\x52\x4f\x4d\x41\x47\x45",
	"\x57\x48\x45\x52\x45\x41\x53",
}

/*!rules:re2c:backtrack
	"ab"                         { return 1, yycursor }
	"abcd"                       { return 2, yycursor }
	"abcdefgh"                   { return 3, yycursor }
	"abcdefghijk"                { return 4, yycursor }
	"abcx"                       { return 5, yycursor }
	"bcdefghi"                   { return 6, yycursor }
	"xyzzyxyzzy"                 { return 7, yycursor }
	"q" [0-9]+ "end"             { return 8, yycursor }
	*                            { return 0, yycursor }
*/

func backtrack_0(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:backtrack
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 0;
		re2c:YYCTYPE = byte;
	*/
}

func backtrack_1(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:backtrack
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 1;
		re2c:YYCTYPE = byte;
	*/
}

var backtrack_words = []string{
	"\x61\x62",
	"\x61\x62\x63\x64",
	"\x61\x62\x63\x64\x65\x66\x67\x68",
	"\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b",
	"\x61\x62\x63\x78",
	"\x62\x63\x64\x65\x66\x67\x68\x69",
	"\x78\x79\x7a\x7a\x79\x78\x79\x7a\x7a\x79",
	"\x71\x31\x32\x33\x65\x6e\x64",
	"\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a",
	"\x78\x79\x7a\x7a\x79\x78\x79\x7a\x7a",
}

/*!rules:re2c:binary
	"\x89PNG\x0d\x0a\x1a\x0a"    { return 1, yycursor }
	"GIF89a"                     { return 2, yycursor }
	"\xef\xbb\xbf"               { return 3, yycursor }
	"PK\x03\x04"                 { return 4, yycursor }
	"%PDF-1."                    { return 5, yycursor }
	"\xff\xd8\xff\xe0"           { return 6, yycursor }
	*                            { return 0, yycursor }
*/

func binary_0(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:binary
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 0;
		re2c:YYCTYPE = byte;
	*/
}

func binary_1(yyinput string) (int, int) {
	yycursor, yymarker := 0, 0
	_ = yymarker
	/*!use:re2c:binary
		re2c:api = simple;
		re2c:yyfill:enable = 0;
		re2c:vectorize:linear = 1;
		re2c:YYCTYPE = byte;
	*/
}

var binary_words = []string{
	"\x89\x50\x4e\x47\x0d\x0a\x1a\x0a",
	"\x47\x49\x46\x38\x39\x61",
	"\x47\x49\x46\x38\x37\x61",
	"\xef\xbb\xbf",
	"\x50\x4b\x03\x04",
	"\x25\x50\x44\x46\x2d\x31\x2e",
	"\xff\xd8\xff\xe0",
	"\xff\xd8\xff\xe1",
}

var failures, checks int

func check1(f, g func(string) (int, int), in string) {
	r1, c1 := f(in)
	r2, c2 := g(in)
	checks++
	if r1 != r2 || c1 != c2 {
		fmt.Fprintf(os.Stderr, "mismatch on %q: rule %d/%d, cursor %d/%d\n", in, r1, r2, c1, c2)
		failures++
	}
}

func check(f, g func(string) (int, int), in string) {
	check1(f, g, in+"\x00")
	pad := in
	for len(pad) < 16 {
		pad += "a"
	}
	check1(f, g, in+"\x00"+pad[:16])
}

func checkSet(f, g func(string) (int, int), ws []string) {
	alt := []byte{0x00, 0x01, 'a', 'z', 'A', 'Z', ':', '-', ' ', '_', '0', 0x7f, 0x80, 0xfe, 0xff}
	for _, w := range ws {
		for i := 0; i <= len(w); i++ {
			check(f, g, w[:i])
			for _, a := range alt {
				check(f, g, w[:i]+string([]byte{a}))
			}
			if i < len(w) {
				check(f, g, w[:i]+string([]byte{w[i] + 1}))
				check(f, g, w[:i]+string([]byte{w[i] - 1}))
			}
		}
		for _, v := range ws {
			check(f, g, w+v)
		}
	}
}

func main() {
	checkSet(headers_0, headers_1, headers_words)
	checkSet(keywords_0, keywords_1, keywords_words)
	checkSet(backtrack_0, backtrack_1, backtrack_words)
	checkSet(binary_0, binary_1, binary_words)
	if failures > 0 {
		panic(fmt.Sprintf("%d of %d checks failed", failures, checks))
	}
}
