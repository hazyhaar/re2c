//go:generate re2go $INPUT -o $OUTPUT -i --api simple --simd
//go:build goexperiment.simd && (amd64 || arm64)

package main

import (
	"fmt"
)

// LexString scanne une tranche d'octets délimitée par des guillemets avec Fast-Forward SIMD.
// Accepte []byte (standard zéro-allocation Go) et retourne la longueur du contenu extrait ou -1.
func LexString(yyinput []byte) int {
	var yycursor int
	yylimit := len(yyinput)

	if yycursor >= yylimit || yyinput[yycursor] != '"' {
		return -1
	}
	yycursor++
	start := yycursor

	for {
		/*!re2c
		re2c:YYCTYPE = byte;
		re2c:yyfill:enable = 0;
		re2c:simd = 1;

		*               { return -1 }
		"\""            { return yycursor - start - 1 }
		"\\"[\\"/bfnrt] { continue }
		[^"\\\x00\n\r]+ { continue }
		*/
	}
}

func main() {
	sample := []byte("\"Identifiant_Systeme_Tres_Long_Sans_Echappement_Pour_Prouver_Le_Debit_SIMD\"\x00")
	length := LexString(sample)
	fmt.Printf("Longueur extraite : %d\n", length)
}
