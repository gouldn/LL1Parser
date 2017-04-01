// Quick, naive, thrown-together scanner for CS254 parser generator assignment.
// Recognizes a variety of useful tokens, for no particular language.
// Michael L. Scott, Sept. 2008.

#include <string>
using std::string;

namespace scanner {

enum tok_num {
    undef           =   0,  // placeholder; value 0 not used
    tok_eof         =   1,  // end of file
    ident           =   2,  // letter ( letter | '_' | digit )*
        rw_array    =   3,  // "array"
        rw_begin    =   4,  // "begin"
        rw_do       =   5,  // "do"
        rw_else     =   6,  // "else"
        rw_end      =   7,  // "end"
        rw_float    =   8,  // "float"
        rw_for      =   9,  // "for"
        rw_if       =  10,  // "if"
        rw_int      =  11,  // "int"
        rw_proc     =  12,  // "proc"
        rw_read     =  13,  // "read"
        rw_real     =  14,  // "real"
        rw_then     =  15,  // "then"
        rw_trunc    =  16,  // "trunc"
        rw_while    =  17,  // "while"
        rw_write    =  18,  // "write"
    lit_int         =  19,  // digit+
    lit_real        =  20,  // digit+ '.' digit+
    becomes         =  21,  // ":="
    op_add          =  22,  // "+"
    op_sub          =  23,  // "-"
    op_mul          =  24,  // "*"
    op_div          =  25,  // "/"
    lparen          =  26,  // "("
    rparen          =  27,  // ")"
    lbrac           =  28,  // "["
    rbrac           =  29,  // "]"
    comma           =  30,  // ","
    semic           =  31,  // ";"
    colon           =  32,  // ":"
    op_lt           =  33,  // "<"
    op_gt           =  34,  // ">"
    op_le           =  35,  // "<="
    op_ge           =  36,  // ">="
    op_eq           =  37,  // "=="
    op_ne           =  38,  // "!="
    tok_error       =  39   // not a valid token
};

static const tok_num first_rw = rw_array;
static const tok_num last_rw  = rw_write;

struct token {
    tok_num num;
    const char* name;   // generic name of this sort of token
    string  image;      // printable image of this particular token
    int     line;       // location of token;
    int     column;     // for error messages
};

extern token scan();

}   // end namespace scanner
