#include "tokenizer.h"
#include <string.h>
#include <stdio.h>

int IsWhiteSpace(char c) {
    return c == ' '  ||
           c == '\t' ||
           c == '\n' ||
           c == '\r';
}

int IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int IsDigit(char c) {
    return (c >= '0' && c <= '9');
}

static const Token EOF_Token = {
    .size = 0,
    .start = 0,
    .t = TOKEN_EOF,
};

//match keywords
TokenType MatchType(Token t) {
    if (memcmp(t.start, "struct", t.size) == 0) {
        return TOKEN_STRUCT;
    }
    if (memcmp(t.start, "typedef", t.size) == 0) {
        return TOKEN_TYPEDEF;
    }
    if (memcmp(t.start, "const", t.size) == 0) {
        return TOKEN_CONST;
    }

    return TOKEN_IDENTIFIER;
}

int IsEOF(Tokenizer* t) {
    return ((t->At - t->data) >= t->size) ||
            !t->At[0];
}

Token PullToken(Tokenizer* t) {

    //eats whitespace
    while (
            IsWhiteSpace(t->At[0]) || t->At[0] == '#' || 
            (t->At[0] == '/' && t->At[1] == '/') 
           )  {
        if (t->At[0] == '#') {
            while (!IsEOF(t) && t->At[0] != '\n') t->At++;
        }
        if (t->At[0] == '/' && t->At[1] == '/') {
            while (!IsEOF(t) && t->At[0] != '\n') t->At++;
        }
        t->At++;
    }

    if (IsEOF(t)) {
        return EOF_Token;
    }

    Token tok = {0};
    if (IsAlpha(t->At[0]) || t->At[0] == '_') {
        tok.start = t->At;
        while (IsAlpha(t->At[0]) || IsDigit(t->At[0]) || t->At[0] == '_') t->At++;
        if (IsEOF(t)) return EOF_Token;

        tok.size = t->At - tok.start;
        tok.t = MatchType(tok);
        return tok;

    } else if (IsDigit(t->At[0])) {

        tok.start = t->At;

        while (IsDigit(t->At[0])) t->At++;
        if (IsEOF(t)) return EOF_Token;

        tok.size = t->At - tok.start;
        tok.t = TOKEN_NUM;
        return tok;
    } 

    t->At++;
    return (Token){
        .start = t->At - 1,
        .size = 1,
        .t = ((t->At) - 1)[0]
    };
}

Token GetToken(Tokenizer* t, uint32_t prev) {
    //512 token buffer
    static Token buffer[512] = {0};
    static uint32_t top = 0;
    
    if (prev == 0) {
        buffer[top] = PullToken(t);
        top = (top + 1) % 512;
        //return buffer[top - 1];
    } 

    uint32_t idx;
    if (prev && top >= prev) {
        idx = (((int32_t)top - prev)) % 512;
    } else {
        //printf("default\n");
        idx = (top + 511) % 512;
    }
    //printf("Dist: %d\n", top - idx);
    //printf("top: %d\n", top);
    return buffer[idx];
}
