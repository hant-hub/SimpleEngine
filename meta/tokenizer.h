#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stdint.h>


typedef enum TokenType {
    TOKEN_EOF = 128,
    TOKEN_STRUCT,
    TOKEN_TYPEDEF,
    TOKEN_CONST,
    TOKEN_IDENTIFIER,
    TOKEN_NUM,
} TokenType;


typedef struct Tokenizer {
    uint32_t size;
    const char* data;
    char* At;
} Tokenizer;

typedef struct Token {
    char* start;
    TokenType t;
    uint32_t size;
} Token;

Token GetToken(Tokenizer* t, uint32_t prev);


#endif
