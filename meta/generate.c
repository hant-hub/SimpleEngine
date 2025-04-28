#include "generate.h"
#include "platform/include/platform.h"
#include "symboltable.h"




void GenerateTypes(sp_file f, SymbolTable* s) {
    sp_fPrintf(f, "#ifndef SE_GENERATED_TYPES_H\n");
    sp_fPrintf(f, "#define SE_GENERATED_TYPES_H\n\n");

    sp_fPrintf(f, "typedef enum SE_meta_type {\n");
    for (int i = 0; i < s->tsize; i++) {
        sp_fPrintf(f, "\tMeta_Type_%s,\n", GetName(s, s->typenames[i]));
    }
    for (int i = 0; i < s->ssize; i++) {
        sp_fPrintf(f, "\tMeta_Type_%s\n", GetName(s, s->structs[i].name));
    }
    sp_fPrintf(f, "} SE_meta_type;\n\n");

    sp_fPrintf(f, "#endif\n");
}

void GenerateStructData(sp_file f, SymbolTable* s) {



    for (int i = 0; i < s->ssize; i++) {
        char* structname = GetName(s, s->structs[i].name);
        sp_fPrintf(f, "static SE_struct_member Meta_Def_%s[] = {\n", structname);
        for (int j = 0; j < s->structs[i].length; j++) {
            Variable v = s->variables[j + s->structs[i].start];
            char* vname = GetName(s, v.name);
            char* tname = GetName(s, v.type);
            sp_fPrintf(f, "\t{");
            sp_fPrintf(f, "Meta_Type_%s, ", tname);
            sp_fPrintf(f, "\"%s\", ", vname);
            sp_fPrintf(f, "(u64)&(((%s *)0)->%s), ", structname, vname);
            sp_fPrintf(f, "(u64)sizeof(%s)", tname);
            sp_fPrintf(f, "},\n");
        }
        sp_fPrintf(f, "};\n");
    }

}
