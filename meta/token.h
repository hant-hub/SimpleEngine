#ifndef TOKEN_H
#define TOKEN_H
#include <stdint.h>

#include "symboltable.h"


typedef enum TokenType {
    TOKEN_ID = 128,
    TOKEN_NUM,
    TOKEN_STRUCT,
    TOKEN_TYPEDEF,
    TOKEN_UNION,
    TOKEN_EOF,
} TokenType;


typedef struct Tokenizer {
    char* data;
    uint64_t filesize;
    char* At;
    uint32_t linenumber;
    SymbolTable s;
} Tokenizer;

typedef struct Token {
    TokenType t;
    int32_t name;
    uint32_t line;
    uint32_t idx; //used for extra metadata
} Token;

Token GetToken(Tokenizer* t);
const char* TokenTypeToName(Token* t);

#endif
