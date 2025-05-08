#include "generate.h"
#include "platform/include/platform.h"
#include "symboltable.h"
#include "token.h"
#include "parser.h"
#include "string.h"

char* files[] = {
    "math/vector.h",
    "../tests/init.h",
};

char* BaseTypes[] = {
    "u32",
    "i32",
    "f32",
};

int main(int argc, char* argv[]) {


    spf_metadata filedata;
    sp_file file; 
    void* data;
    Tokenizer tok = {0};

    sp_Chdir("include");

    for (int i = 0; i < sizeof(BaseTypes)/sizeof(BaseTypes[0]); i++) {
        uint32_t n = InsertSymbol(&tok.s, BaseTypes[i], strlen(BaseTypes[i]));
        PushType(&tok.s, n);
    }
    
    for (int i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
        file = sp_OpenFile(files[i], spf_READ, 0, &filedata);
        data = sp_MemMapFile(file, filedata, spm_READ | spm_PRIVATE);
        tok.data = data;
        tok.At = data;
        tok.filesize = filedata.size;
        tok.linenumber = 0;

        ParseFile(&tok);

        sp_UnMapFile(data, filedata);
        sp_CloseFile(file);
    }

    sp_file typefile = sp_OpenFile("generated/generated_types.h", spf_WRITE, spf_TRUNC | spf_CREAT, &filedata);
    sp_file structfile = sp_OpenFile("generated/generated_struct.h", spf_WRITE, spf_TRUNC | spf_CREAT, &filedata);

    GenerateTypes(typefile, &tok.s);

    sp_fPrintf(structfile, "#ifndef SE_GENERATED_STRUCT_H\n");
    sp_fPrintf(structfile, "#define SE_GENERATED_STRUCT_H\n\n");

    for (int i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
        sp_fPrintf(structfile, "#include <%s>\n", files[i]);
    }

    sp_fPrintf(structfile, "\n");
    sp_fPrintf(structfile, "#include \"generated_static.h\"\n");
    sp_fPrintf(structfile, "#include \"generated_types.h\"\n\n");

    GenerateStructData(structfile, &tok.s);


    sp_fPrintf(structfile, "\n");
    sp_fPrintf(structfile, "#endif\n");

    sp_CloseFile(typefile);
    sp_CloseFile(structfile);

    sp_HeapFree(tok.s.buffer.sbuf);
    sp_HeapFree(tok.s.structs);
    sp_HeapFree(tok.s.typenames);
    sp_HeapFree(tok.s.values);
    sp_HeapFree(tok.s.types);
    sp_HeapFree(tok.s.idx);
    sp_HeapFree(tok.s.variables);
    return 0;
}
