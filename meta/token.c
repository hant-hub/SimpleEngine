#include "token.h"
#include "platform/include/platform.h"
#include "symboltable.h"
#include <stdlib.h>
#include <platform.h>
#include <string.h>




int IsAlpha(char c) {
    return (c <= 'Z' && c >= 'A') ||
           (c <= 'z' && c >= 'a');
}

int IsDigit(char c) {
    return (c <= '9' && c >= '0');
}

int IsWhiteSpace(char c) {
    return (c == ' ') ||
           (c == '\t') ||
           (c == '\r');
}

void EatCStyleComment(Tokenizer* t) {
    while (1) {
        if (t->At[0] == '*' && t->At[1] == '/') break;
        t->At++;
    }
}

void EatCppStyleComment(Tokenizer* t) {
    while (t->At[0] != '\n') t->At++;
}

void EatDirectives(Tokenizer* t) {
    while (t->At[0] != '\n') t->At++;
}

void EatWhiteSpace(Tokenizer* t) {
    while (1) {
        if (IsWhiteSpace(t->At[0])) {
            t->At++;
            continue;
        }
        if (t->At[0] == '\n') {
            t->linenumber++;
            t->At++;
            continue;
        }
            
        if (t->At[0] == '/') {
            if (t->At[1] == '*') {
                EatCStyleComment(t);
                continue;
            } else if (t->At[1] == '/') {
                EatCppStyleComment(t);
                continue;
            }
        }

        if (t->At[0] == '#') {
            EatDirectives(t);
            continue;
        }
        break;
    }
}

Token CheckKeyword(Token t, SymbolTable* s) {
    if (strcmp(GetName(s, t.name), "struct") == 0) {
        t.t = TOKEN_STRUCT;
        uint32_t entry = GetEntry(s, t.name);
        s->types[entry] = SYMBOL_KEYWORD;
        return t;
    }

    if (strcmp(GetName(s, t.name), "typedef") == 0) {
        t.t = TOKEN_TYPEDEF;
        uint32_t entry = GetEntry(s, t.name);
        s->types[entry] = SYMBOL_KEYWORD;
        return t; 
    }

    if (strcmp(GetName(s, t.name), "union") == 0) {
        t.t = TOKEN_UNION;
        uint32_t entry = GetEntry(s, t.name);
        s->types[entry] = SYMBOL_KEYWORD;
        return t; 
    }

    uint32_t entry = GetEntry(s, t.name);
    s->types[entry] = SYMBOL_NAME;
    t.t = TOKEN_ID;
    return t; 
}

Token GetToken(Tokenizer* t) {
    
    EatWhiteSpace(t);
    if (t->At - t->data >= t->filesize) { 
        return (Token){
            .t = TOKEN_EOF
        };
    }

    Token out = {0};
    //ID or keyword
    if (IsAlpha(t->At[0]) || t->At[0] == '_') {
        char* pos = t->At++;
        while (IsAlpha(t->At[0]) ||
                IsDigit(t->At[0]) ||
                t->At[0] == '_' ||
                t->At[0] == '-'
                ) {
            t->At++;
        }
        uint64_t size = t->At - pos;
        out.name = InsertSymbol(&t->s, pos, size);
        out = CheckKeyword(out, &t->s);
        return out;
    }

    //Number of some kind
    if (IsDigit(t->At[0])) {
        char* pos = t->At++;
        while (IsDigit(t->At[0])) {
            t->At++;
        }
        uint64_t size = t->At - pos;
        out.name = InsertSymbol(&t->s, pos, size);
        out.t = TOKEN_NUM;
        return out;
    }

    out = (Token){t->At[0], -1, t->linenumber, 1};
    t->At++;
    return out;
}

const char* TokenTypeNames[] = {
    "ID",
    "NUM",
    "STRUCT",
    "TYPEDEF",
    "EOF"
};

const char* TokenTypeToName(Token* t) {
    if (t->t < 128) {
        return "Literal";
    }
    return TokenTypeNames[t->t - 128];
}
