#include "platform/include/platform.h"
#include "token.h"
#include "parser.h"



int main(int argc, char* argv[]) {

    spf_metadata filedata;
    sp_file file = sp_OpenFile("include/platform/wayland/wayland.h", spf_READ, 0, &filedata);
    void* data = sp_MemMapFile(file, filedata, spm_READ | spm_PRIVATE);

    Tokenizer tok = (Tokenizer){
        .data = data,
        .filesize = filedata.size,
        .At = data,
        .linenumber = 0,
    };
    ParseFile(&tok);

    sp_UnMapFile(data, filedata);
    sp_CloseFile(file);

    file = sp_OpenFile("include/math/vector.h", spf_READ, 0, &filedata);
    data = sp_MemMapFile(file, filedata, spm_READ | spm_PRIVATE);

    tok.data = data;
    tok.At = data;
    tok.filesize = filedata.size;
    tok.linenumber = 0;
    ParseFile(&tok);

    sp_UnMapFile(data, filedata);
    sp_CloseFile(file);
    return 0;
}
