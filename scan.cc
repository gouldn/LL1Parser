//For CSC 435 Programming Languages, project 2 part 1. 
//A naive scanner. 

#include <iostream>
using std::cin;
#include <assert.h>
#include <ctype.h>      // isspace, isdigit, isalpha, etc.
#include <string.h>
#include <stdio.h>

#include "scan.h"

namespace scanner {

const char *token_names[] = {
    "undef",
    "tok_eof",
    "ident",
        "rw_array",
        "rw_begin",
        "rw_do",
        "rw_else",
        "rw_end",
        "rw_float",
        "rw_for",
        "rw_if",
        "rw_int",
        "rw_proc",
        "rw_read",
        "rw_real",
        "rw_then",
        "rw_trunc",
        "rw_while",
        "rw_write",
    "lit_int",
    "lit_real",
    "becomes",
    "op_add",
    "op_sub",
    "op_mul",
    "op_div",
    "lparen",
    "rparen",
    "lbrac",
    "rbrac",
    "comma",
    "semic",
    "colon",
    "op_lt",
    "op_gt",
    "op_le",
    "op_ge",
    "op_eq",
    "op_ne",
    "tok_error"
};

static int cur_line = 1;                // where we are now
static int cur_col  = 0;

static int tok_line;                    // where current token started
static int tok_col;

static const int TOKEN_MAX = 100;
static char token_image[TOKEN_MAX];     // text of current token
static int image_index;                 // next free slot in token_image

static int cur_char = ' ';
    // character most recently read
    // not yet in token_image
    // int width accommodates EOF 

static inline void nextchar() {
    assert(image_index < TOKEN_MAX);
    token_image[image_index++] = cur_char;
    cur_char = cin.get();
    cur_col++;
    if (cur_char == '\n') {
        cur_line++;
        cur_col = 0;
    }
}

static inline token make_token(tok_num t) {
    token rtn;
    rtn.num = t;
    assert(image_index < TOKEN_MAX);
    token_image[image_index] = 0;
    rtn.name = token_names[t];
    rtn.image = string(token_image);
    rtn.line = tok_line;
    rtn.column = tok_col;
    return rtn;
}

/*
 *  Scans input, matching tokens. Stores text in
 *  token_image, converts to string inside token returned.
 */
token scan() {
    image_index = 0;

    // skip white space 
    while (isspace(cur_char)) {
        cur_char = cin.get();
        cur_col++;
        if (cur_char == '\n') {
            cur_line++;
            cur_col = 0;
        }
    }
    tok_line = cur_line;  tok_col = cur_col;

    if (cur_char == EOF) {                     // end of file
        return make_token(tok_eof);
    }

    if (isalpha(cur_char)) {                   // identifier
        do {
            nextchar();
        } while (isdigit(cur_char) || isalpha(cur_char) || cur_char == '_');

        // check for reserved words
        token_image[image_index] = 0;

        for (int w = first_rw; w <= last_rw; w++) {
            // This is a bad way to do this!
            // Should replace linear search with hash table.
            if (!strcmp(token_image, &token_names[w][3])) {
                return make_token(tok_num(w));
            }
        }
        // else not a reserved word - it's an identifier
        return make_token(ident);
    }

    else if (isdigit(cur_char)) {              // literal
        tok_num tok = lit_int;
        do {
            nextchar();
        } while (isdigit(cur_char));
        if (cur_char == '.') {
            nextchar();
            if (!isdigit(cur_char)) {
                return make_token(tok_error);
            }
            tok = lit_real;
            do {
                nextchar();
            } while (isdigit(cur_char));
        }
        return make_token(tok);

    } else switch (cur_char) {                 // misc. other cases
        case ':':
            nextchar();
            if (cur_char != '=') {
                return make_token(colon);
            } else {
                nextchar();
                return make_token(becomes);
            }
            break;
        case '<':
            nextchar();
            if (cur_char != '=') {
                return make_token(op_lt);
            } else {
                nextchar();
                return make_token(op_le);
            }
            break;
        case '>':
            nextchar();
            if (cur_char != '=') {
                return make_token(op_gt);
            } else {
                nextchar();
                return make_token(op_ge);
            }
            break;
        case '=':
            nextchar();
            if (cur_char != '=') {
                return make_token(tok_error);
            } else {
                nextchar();
                return make_token(op_eq);
            }
            break;
        case '!':
            nextchar();
            if (cur_char != '=') {
                return make_token(tok_error);
            } else {
                nextchar();
                return make_token(op_ne);
            }
            break;
        case '+': nextchar(); return make_token(op_add);
        case '-': nextchar(); return make_token(op_sub);
        case '*': nextchar(); return make_token(op_mul);
        case '/': nextchar(); return make_token(op_div);
        case '(': nextchar(); return make_token(lparen);
        case ')': nextchar(); return make_token(rparen);
        case '[': nextchar(); return make_token(lbrac);
        case ']': nextchar(); return make_token(rbrac);
        case ',': nextchar(); return make_token(comma);
        case ';': nextchar(); return make_token(semic);
        default:  nextchar(); return make_token(tok_error);
    }
}
}   // end namespace scanner
