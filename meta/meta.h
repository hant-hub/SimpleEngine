#ifndef META_H
#define META_H


#include <stdint.h>
typedef struct File {
    char* data;
    uint32_t size;
    int fd;
} File;




#endif
