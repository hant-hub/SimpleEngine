#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

#include <stdint.h>

typedef enum SymbolType {
    SYMBOL_TYPE,
    SYMBOL_NAME,
    SYMBOL_KEYWORD,
    SYMBOL_STRUCT,
} SymbolType;

typedef struct Name {
    int32_t name; //index into string buffer
    uint32_t hash; //stored for fast checking and rehashing

} Name;

typedef struct Variable {
    uint32_t name;
    //entry in type stack
    uint32_t indirection;
    uint32_t type;
} Variable;

typedef struct StructDef {
    uint32_t name;
    uint32_t start;
    uint32_t length;
} StructDef;

typedef struct Type {
    uint32_t name;
    uint32_t size;
} Type;

typedef struct StringBuffer {
    uint32_t cap;
    uint32_t size;
    char* sbuf;
} StringBuffer;

typedef struct SymbolTable {
    StringBuffer buffer;
    uint64_t sbuf_size;

    //symbol hash table
    uint32_t cap;
    uint32_t size;
    Name* values;
    int32_t* idx;
    SymbolType* types;

    //variable definitions
    uint32_t vcap;
    uint32_t vsize;
    Variable* variables;

    //struct definitions
    uint32_t scap;
    uint32_t ssize;
    StructDef* structs;

    //types
    uint32_t tcap;
    uint32_t tsize;
    uint32_t* typenames;

} SymbolTable;

uint32_t PushSymbol(const char* string, uint32_t size, SymbolTable* s);
char* GetName(SymbolTable* t, uint32_t name);

uint32_t InsertSymbol(SymbolTable* t, const char* key, uint32_t size);
int32_t GetEntry(SymbolTable* table, uint32_t key);

uint32_t PushVariable(SymbolTable* t, uint32_t type, uint32_t name);
void PushStructDef(SymbolTable* t, uint32_t name, uint32_t start, uint32_t size);
void PushType(SymbolTable* t, uint32_t name);

#endif
