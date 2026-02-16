#ifndef SE_INTROSPECT_H
#define SE_INTROSPECT_H

#include "cutils.h"

enum SEVarType : u32 {
    SE_VAR_TYPE_U32,
    SE_VAR_TYPE_F32,
    SE_VAR_TYPE_V2F,
    SE_VAR_TYPE_V3F,
};

enum SEVarType : u32;
typedef enum SEVarType SEVarType;

typedef struct SEStructSpec {
    SString name; 
    SEVarType type;
    u32 size;
    u32 offset;
} SEStructSpec;



//markers for preprocessor
#define SEStartParse()
#define SEEndParse()


#endif
