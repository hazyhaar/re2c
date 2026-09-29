# re2py $INPUT -o $OUTPUT
/*!re2c
    re2c:keywords = 1;
    re2c:keywords:token = "tok";
    "if" { return 1 }
    [a-z]+ { return 2 }
*/
