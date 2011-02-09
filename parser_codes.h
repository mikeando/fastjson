// This code is a modification of JSON_parser.c from http://fara.cs.uni-potsdam.de/~jsg/json_parser/JSON_parser.c
// Which is licensed as:
/*
Copyright (c) 2005 JSON.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


enum classes {
    C_SPACE,  /* space */
    C_WHITE,  /* other whitespace */
    C_LCURB,  /* {  */
    C_RCURB,  /* } */
    C_LSQRB,  /* [ */
    C_RSQRB,  /* ] */
    C_COLON,  /* : */
    C_COMMA,  /* , */
    C_QUOTE,  /* " */
    C_BACKS,  /* \ */
    C_SLASH,  /* / */
    C_PLUS,   /* + */
    C_MINUS,  /* - */
    C_POINT,  /* . */
    C_DIGIT,  /* 0123456789 */
    C_HEX,    /* abcdef ABCDEF */
    C_ETC,    /* everything else */
    C_STAR,   /* * */   
    C_CTRL,   /* non-whitespace control characters - not valid in JSON*/
    NR_CLASSES
};




static const signed char ascii_class[128] = {
/*
    This array maps the 128 ASCII characters into character classes.
    The remaining Unicode characters should be mapped to C_ETC.
    Non-whitespace control characters are errors.
*/
    C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,
    C_CTRL,  C_WHITE, C_WHITE, C_CTRL,  C_CTRL,  C_WHITE, C_CTRL,  C_CTRL,
    C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,
    C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,  C_CTRL,

    C_SPACE, C_ETC,   C_QUOTE, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_STAR,  C_PLUS,  C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT, C_DIGIT, C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

    C_ETC,   C_HEX,   C_HEX,   C_HEX,   C_HEX,   C_HEX,   C_HEX,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LSQRB, C_BACKS, C_RSQRB, C_ETC,   C_ETC,

    C_ETC,   C_HEX,   C_HEX,   C_HEX,   C_HEX,   C_HEX,   C_HEX,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LCURB, C_ETC,   C_RCURB, C_ETC,   C_ETC
};



