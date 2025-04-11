#define _POSIX_C_SOURCE 200809L

#include <stdio.h>


#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "tokenizer.h"
#include "parser.h"
#include "meta.h"
#include "generate.h"

char* files[] = {
    "math/vector.h",
    "platform/wayland/wayland.h"
};

//TODO(ELI): Make this cross platform

//TODO(ELI): Implement shader reflection

//TODO(ELI): Investigate Custom Shader preprocessor?

int main(int argc, char* argv[]) {
    
    FILE* struct_defs = fopen("include/generated/generated_struct.h","w+");
    FILE* type_defs = fopen("include/generated/generated_types.h","w+");

    fprintf(struct_defs, "#ifndef SE_GENERATED_DEF_H\n");
    fprintf(struct_defs, "#define SE_GENERATED_DEF_H\n");

    StructData* head = NULL;
    for (int i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
        File f; 
        char temp[1024] = {0};
        snprintf(temp, sizeof(temp), "include/%s", files[i]);
        f.fd = open(temp, O_RDONLY); 
        head = GetStructData(&f, head);
        fprintf(struct_defs, "#include <%s>\n", files[i]);
    }

    fprintf(struct_defs, "#include <util.h>\n");
    fprintf(struct_defs, "#include <generated/generated_static.h>\n");
    GenerateStructDefSource(struct_defs, head);
    fprintf(struct_defs, "#endif\n");

    GenerateStructDefHeader(type_defs, head);

    //FreeStructData(head, f); 

    fclose(struct_defs);





    return 0;
}
