//go:generate re2go $INPUT -o $OUTPUT -i --api simple
package main

import "fmt"

// Keyword tables for keyword rules subsumed by the identifier rule.
/*!keywords:re2c*/

const (
	ID = iota
	NUM
	SPACE
	END
	ERR
	IN_PREFIX
	KW_X
	KW_AB
	KW_ABC
	KW_FROM
	KW_WHERE
	KW_SELECT
	KW_BETWEEN
	KW_DISTINCT
	KW_EXCLUSIVE
	KW_CURRENT_DATE
	KW_CURRENT_TIME
	KW_AUTO_INCREMENT
	KW_TRANSACTION_ID
	KW_INT
	KW_CURRENT_TIMESTAMP
)

func lex(yyinput []byte, yycursor int) (int, int) {
	var yymarker int
	_ = yymarker
	tok := yycursor
	/*!re2c
	re2c:yyfill:enable = 0;
	re2c:YYCTYPE = byte;
	re2c:keywords = 1;
	re2c:keywords:token = tok;

	"\x00"                 { return END, yycursor }
	[ ]+                   { return SPACE, yycursor }
	[0-9]+                 { return NUM, yycursor }
	"x"                    { return KW_X, yycursor }
	"ab"                   { return KW_AB, yycursor }
	"abc"                  { return KW_ABC, yycursor }
	"from"                 { return KW_FROM, yycursor }
	"where"                { return KW_WHERE, yycursor }
	"select"               { return KW_SELECT, yycursor }
	"between"              { return KW_BETWEEN, yycursor }
	"distinct"             { return KW_DISTINCT, yycursor }
	"exclusive"            { return KW_EXCLUSIVE, yycursor }
	"current_date"         { return KW_CURRENT_DATE, yycursor }
	"current_time"         { return KW_CURRENT_TIME, yycursor }
	"auto_increment"       { return KW_AUTO_INCREMENT, yycursor }
	"transaction_id"       { return KW_TRANSACTION_ID, yycursor }
	// "int" is also matched by the next rule, so it must stay in the DFA
	"int"                  { return KW_INT, yycursor }
	"i" [a-z]+             { return IN_PREFIX, yycursor }
	// longer than 16 characters, stays in the DFA
	"current_timestamp"    { return KW_CURRENT_TIMESTAMP, yycursor }
	[a-z_][a-z0-9_]*       { return ID, yycursor }
	*                      { return ERR, yycursor }
	*/
}

func main() {
	tests := []struct {
		str  string
		kind int
	}{
		{"x", KW_X}, {"xx", ID}, {"a", ID}, {"ab", KW_AB}, {"abc", KW_ABC}, {"abcd", ID},
		{"fro", ID}, {"from", KW_FROM}, {"froma", ID}, {"where", KW_WHERE}, {"wherex", ID},
		{"select", KW_SELECT}, {"selec", ID}, {"selecta", ID}, {"between", KW_BETWEEN},
		{"distinct", KW_DISTINCT}, {"distinc", ID}, {"distincts", ID},
		{"exclusive", KW_EXCLUSIVE}, {"exclusiv", ID}, {"exclusives", ID},
		{"current_date", KW_CURRENT_DATE}, {"current_time", KW_CURRENT_TIME},
		{"current_dat", ID}, {"current_datex", ID}, {"current_timestamp", KW_CURRENT_TIMESTAMP},
		{"current_timestampx", ID}, {"current_times", ID},
		{"auto_increment", KW_AUTO_INCREMENT}, {"auto_incremenu", ID},
		{"transaction_id", KW_TRANSACTION_ID}, {"transaction_ids", ID},
		{"int", KW_INT}, {"into", IN_PREFIX}, {"in", IN_PREFIX}, {"i", ID},
		{"abcdefghijklmnop", ID}, {"abcdefghijklmnopq", ID}, {"42", NUM}, {"-", ERR},
	}
	for _, t := range tests {
		in := []byte(t.str + " \x00")
		kind, end := lex(in, 0)
		if kind != t.kind || end != len(t.str) {
			panic(fmt.Sprintf("%q: got kind %d end %d, want kind %d end %d",
				t.str, kind, end, t.kind, len(t.str)))
		}
	}
	// A token at the very end of the input: the lookup must not read past it.
	for _, t := range tests {
		in := append([]byte(t.str), 0)
		in = in[:len(in):len(in)]
		if kind, end := lex(in, 0); kind != t.kind || end != len(t.str) {
			panic(t.str)
		}
	}
}
