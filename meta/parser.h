#ifndef PARSER_H
#define PARSER_H
#include "meta.h"
#include <stdint.h>

typedef struct MetaType {
    char* basename;
    uint32_t nameSize;
    uint32_t isPointer;
    uint32_t isStruct;
} MetaType;

//may store more info later
typedef struct StructMember {
    char* name;
    uint32_t nameSize;
    
    MetaType t;
} StructMember;

//variable size
typedef struct StructData {
    struct StructData* next;
    char* name;
    uint32_t nameSize;
    uint32_t numMembers;
    uint32_t isTypedef;
    uint32_t skipdef;
    StructMember* members;
} StructData;

StructData* GetStructData(File* f, StructData* head);
void FreeStructData(StructData* head, File f);

#endif
