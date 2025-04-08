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


int main(int argc, char* argv[]) {
    
    File f; 
    f.fd = open("include/render/render.h", O_RDONLY); 
    StructData* head = GetStructData(&f);

    GenerateStructDefHeader("render/render.h", head);
    GenerateStructDefSource("render/render.h", head);

    FreeStructData(head, f); 




    return 0;
}
