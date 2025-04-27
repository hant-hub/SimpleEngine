#ifndef PARSER_H
#define PARSER_H

#include "token.h"


typedef struct MemberDef {
    uint32_t type;  
    uint32_t name;
} MemberDef;

typedef struct MemberTable {
    uint32_t size;
    uint32_t cap;
} MemberTable;




void ParseFile(Tokenizer* t);

#endif
