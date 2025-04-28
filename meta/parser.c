#include "parser.h"
#include "platform/include/platform.h"
#include "symboltable.h"
#include "token.h"
#include <string.h>
#include <sys/select.h>


void ParseStruct(Tokenizer* t) {
    //either struct or typedef
    Token name = GetToken(t); 
    name = GetToken(t);
    if (strcmp(&t->s.buffer.sbuf[name.name], "struct") == 0) {
        name = GetToken(t);
    }
    uint32_t entry = GetEntry(&t->s, name.name);
    sp_Printf("Name: %s, %d\n", GetName(&t->s, name.name), t->s.types[entry]);

    //Opening bracket
    GetToken(t);
    
    uint32_t start = t->s.vsize;
    uint32_t num = 0;

    //Parse Member defs
    Token type;
    while ((type = GetToken(t)).t != '}') {
        //advance if struct declaration
        if (strcmp(GetName(&t->s, type.name), "struct") == 0) {
            type = GetToken(t);
        }
        Token name = GetToken(t);
        Token end;

        //TODO(ELI): Scheme to support pointer indirection
        //Maybe add indirection count on the variable declartion? 
        while ((end = GetToken(t)).t != ';') {
            name = end;
            end = GetToken(t);
        }

        PushType(&t->s, type.name);
        PushVariable(&t->s, type.name, name.name);
        //sp_Printf("\t%s %s\n", GetName(&t->s, type.name), GetName(&t->s, name.name));
        num++;
    }

    PushStructDef(&t->s, name.name, start, num);

    //Closing Bracket
    return;
}


void ParseFile(Tokenizer* t) {
    Token c;
    while ((c = GetToken(t)).t != TOKEN_EOF) {
        if (c.t != TOKEN_ID) continue;
        if (strcmp(&t->s.buffer.sbuf[c.name], "META_INTROSPECT") == 0) {
            GetToken(t);
            GetToken(t);
            ParseStruct(t);
        } else if (strcmp(GetName(&t->s, c.name), "META_BASE") == 0) {
            GetToken(t);
            GetToken(t);
            //push type here
            Token name;
            while (1) {
                name = GetToken(t);
                if (name.t != TOKEN_UNION && 
                    name.t != TOKEN_STRUCT && 
                    name.t != TOKEN_TYPEDEF) break;
            }
            sp_Printf("Base type: %s\n", GetName(&t->s, name.name));
            PushType(&t->s, name.name);
        }
    }

    //sp_Printf("string: ");
    //for (int i = 0; i < t->s.buffer.size; i++) {
    //    if (!t->s.buffer.sbuf[i]) {
    //        sp_Printf("\n\t");
    //    } else {
    //        sp_Printf("%c", t->s.buffer.sbuf[i]);
    //    }
    //}
    //sp_Printf("\n");

    //sp_Printf("types:\n");
    //for (int i = 0; i < t->s.tsize; i++) {
    //    sp_Printf("\t%s\n", GetName(&t->s, t->s.typenames[i]));
    //}


    //sp_Printf("structs:\n");
    //for (int i = 0; i < t->s.ssize; i++) {
    //    sp_Printf("\t%s %d\n", GetName(&t->s, t->s.structs[i].name), t->s.structs[i].length);
    //    for (int j = 0; j < t->s.structs[i].length; j++) {
    //        sp_Printf("\t\t%s\n", GetName(&t->s, t->s.variables[j + t->s.structs[i].start].name));
    //    }
    //}
}
