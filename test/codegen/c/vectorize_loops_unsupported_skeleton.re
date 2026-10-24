// re2c $INPUT -o $OUTPUT -i --skeleton --vectorize-loops
/*!re2c
    [a-z]+ { return 1; }
    *      { return 0; }
*/
