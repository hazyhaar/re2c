package main

import (
	"testing"
)

func TestKeywords_Parity(t *testing.T) {
	tests := []struct {
		input string
		want  int
	}{
		{"SELECT\x00", TokSelect},
		{"INSERT\x00", TokInsert},
		{"UPDATE\x00", TokUpdate},
		{"DELETE\x00", TokDelete},
		{"FROM\x00", TokFrom},
		{"WHERE\x00", TokWhere},
		{"SELEC\x00", TokUnknown},
		{"SELECTT\x00", TokUnknown},
		{"\x00", TokUnknown},
		{"XYZ\x00", TokUnknown},
	}

	for _, tt := range tests {
		got := LexKeyword(tt.input)
		if got != tt.want {
			t.Errorf("LexKeyword(%q) = %d, attendu %d", tt.input, got, tt.want)
		}
	}
}

func BenchmarkKeywords_Lex(b *testing.B) {
	kw := "SELECT\x00"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		res := LexKeyword(kw)
		if res != TokSelect {
			b.Fatalf("Résultat inattendu")
		}
	}
}
