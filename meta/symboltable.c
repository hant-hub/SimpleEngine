#include "symboltable.h"
#include "platform/include/platform.h"
#include <string.h>
#include <assert.h>

typedef enum HASH_STATE {
    HASH_EMPTY = -2,
    HASH_TOMBSTONE, //currently unused since symbols are not deleted
} HASH_STATE;

char* GetName(SymbolTable* t, uint32_t name) {
    return &t->buffer.sbuf[name];
}


//Murmurhash2
//NOTE(ELI): Possibly create a custom hash function
//for the keyword table, rn this should be acceptable
static uint32_t hash(const char* buffer, uint32_t len) {
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    uint32_t h = 0;

    unsigned char* data = (unsigned char*)buffer;

    while (len >= 4) {
        uint32_t k = *data;

        k *= m;
        k ^= k >> r;
        k *= m;

        k *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
                h *= m;
    }

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}


//Find either the first instance or first avalible 
//slot
int32_t GetEntry(SymbolTable* table, uint32_t key) {
    char* name = GetName(table, key);
    uint32_t idx = hash(name, strlen(name)); 
    idx %= table->cap;

    for (uint32_t i = 0; i < table->cap; i++) {
        //sp_Printf("name: %d\n", table->values[i].name);
        //sp_Printf("idx: %d\n", idx);
        if (table->values[i].name >= 0) {
            if (key == table->values[i].name) {
                return i;
            }
        } else {
            return -1;
        }

        idx += 1;
        idx %= table->cap;
    }
    return -1;
}

uint32_t GetInsertIndex(SymbolTable* table, const char* key, uint32_t size) {
    uint32_t idx = hash(key, size); 
    idx %= table->cap;

    //linear probing 
    for (uint32_t i = 0; i < table->cap; i++) {
        //sp_Printf("name: %d\n", table->values[i].name);
        //sp_Printf("idx: %d\n", idx);
        if (table->values[i].name >= 0) {
            char* tablename = GetName(table, table->values[i].name);
            if (size == strlen(tablename) && memcmp(key, GetName(table, table->values[i].name), size) == 0) {
                //sp_Printf("check: %.*s, %s\n", size, key, GetName(table, table->values[i].name)); 
                //first instance
                return i;
            }
        } else {
            //sp_Printf("empty: %.*s\n", size, key);
            return i;
        }

        idx += 1;
        idx %= table->cap;
    }
    assert(0);
}

uint32_t PushSymbol(const char* string, uint32_t size, SymbolTable* s) {
    if (s->buffer.size + size + 1 > s->buffer.cap) {
        s->buffer.cap = s->buffer.cap ? s->buffer.cap * 2 : 64;
        s->buffer.sbuf = sp_HeapRealloc(s->buffer.sbuf, s->buffer.cap);
    }

    uint32_t index = s->buffer.size;
    memcpy(&s->buffer.sbuf[s->buffer.size], string, size); 
    s->buffer.size += size;
    s->buffer.sbuf[s->buffer.size] = 0;
    s->buffer.size += 1;

    return s->buffer.size - size - 1;
}

void ReSizeTable(SymbolTable* t);

uint32_t InsertSymbol(SymbolTable* t, const char* key, uint32_t size) {
    if (!t->cap) {
        t->cap = 1;
        t->values = sp_HeapAlloc(t->cap * sizeof(Name));
        t->types = sp_HeapAlloc(t->cap * sizeof(SymbolType));
        t->idx = sp_HeapAlloc(t->cap * sizeof(uint32_t));
        t->values[0].name = HASH_EMPTY;
        t->values[0].hash = 0;
        t->types[0] = -1;
        t->idx[0] = 0;
    }

    if ((float)t->size + 1 > (float)t->cap * 0.5) {
        ReSizeTable(t);
        //sp_Printf("resize: %d\n", t->cap);
    }

    uint32_t idx = GetInsertIndex(t, key, size);  

    if (t->values[idx].name < 0) {
        //sp_Printf("insert %.*s %d\n", size, key, idx);
        t->values[idx].name = PushSymbol(key, size, t);
        t->values[idx].hash = hash(key, size);
        t->idx[idx] = -1;
        t->size++;
    }

    return t->values[idx].name;
}

void ReSizeTable(SymbolTable* t) {
    uint32_t new_size = t->cap ? t->cap * 2 : 1;
    Name* new_table = sp_HeapAlloc(new_size * sizeof(Name));
    SymbolType* new_types = sp_HeapAlloc(new_size * sizeof(SymbolType));
    int32_t* new_idx = sp_HeapAlloc(new_size * sizeof(int32_t));

    for (uint32_t i = 0; i < new_size; i++) {
        new_table[i].name = HASH_EMPTY;
        new_table[i].hash = 0;
        new_types[i] = -1;
    }

    for (uint32_t i = 0; i < t->cap; i++) {
        if (t->values[i].name < 0) continue;
        //sp_Printf("old_table: (%d, %d)\n", t->values[i].name, t->values[i].hash);

        uint32_t idx = t->values[i].hash % new_size;
        for (uint32_t j = 0; j < new_size; j++) {
            if (new_table[j].name < 0) {
                new_table[j] = t->values[i];
                new_types[j] = t->types[i];
                new_idx[j] = t->idx[i];
                break;
            }

            idx += 1;
            idx %= t->cap;
        }
    }

    //sp_Printf("new_table:\n\t(%d, %d)\n", new_table[0].name, new_table[0].hash);
    //for (uint32_t i = 1; i < new_size; i++) {
    //    sp_Printf("\t(%d, %d)\n", new_table[i].name, new_table[i].hash);
    //}

    sp_HeapFree(t->values);
    sp_HeapFree(t->types);
    sp_HeapFree(t->idx);
    t->types = new_types;
    t->values = new_table;
    t->idx = new_idx;
    t->cap = new_size;
}

//TODO(ELI): Write variable definition
//Query table for previous name?
//If I add deduplication I would require
//scoping rules, however this isn't a full compiler
//so that seems overkill, should probably just write the
//declartion in
uint32_t PushVariable(SymbolTable* t, uint32_t type, uint32_t name) {
    if (t->vsize + 1 > t->vcap) {
        t->vcap = t->vcap ? t->vcap * 2 : 8;
        t->variables = sp_HeapRealloc(t->variables, sizeof(Variable) * t->vcap);
    }

    t->variables[t->vsize++] = (Variable) {
        .type = type,
        .name = name,
    };
    return t->vsize - 1;
}

void PushStructDef(SymbolTable* t, uint32_t name, uint32_t start, uint32_t size) {
    if (t->ssize + 1 > t->scap) {
        t->scap = t->scap ? t->scap * 2 : 8;
        t->structs = sp_HeapRealloc(t->structs, sizeof(Variable) * t->scap);
    }

    t->structs[t->ssize++] = (StructDef){
        .name = name,
        .start = start,
        .length = size
    };

    uint32_t e = GetEntry(t, name);
    t->idx[e] = t->ssize - 1;
}

//TODO(ELI): Write Type
//Query table for type
void PushType(SymbolTable* t, uint32_t name) {

    uint32_t e = GetEntry(t, name);      
    if (t->idx[e] >= 0) return;

    if (t->tsize + 1 > t->tcap) {
        t->tcap = t->tcap ? t->tcap * 2 : 8;
        t->typenames = sp_HeapRealloc(t->typenames, sizeof(Variable) * t->tcap);
    }
    
    t->idx[e] = t->tsize;
    t->typenames[t->tsize++] = name;
}
