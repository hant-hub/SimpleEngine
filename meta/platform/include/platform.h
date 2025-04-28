#ifndef SIMPLE_PLATFORM_H
#define SIMPLE_PLATFORM_H



#include <stdint.h>
#include <sys/types.h>

//platform dependent------------
typedef int sp_file;
typedef pid_t sp_process;
//------------------------------

typedef uint32_t Bool32;
#define TRUE 1
#define FALSE 0

//required
typedef enum sp_file_mode {
    spf_READ   = 1 << 0,
    spf_WRITE  = 1 << 1,
} sp_file_mode;

//optional
typedef enum sp_file_flags {
    spf_APPEND = 1 << 0,
    spf_TRUNC  = 1 << 1,
    spf_CREAT  = 1 << 2,
} sp_file_flags;

typedef struct spf_metadata {
    uint64_t size;
} spf_metadata;

sp_file sp_OpenFile(const char* file, sp_file_mode mode, sp_file_flags flags, spf_metadata* meta);
void sp_ResizeFile(sp_file file, spf_metadata* meta, uint64_t newsize);
void sp_CloseFile(sp_file file);

typedef enum sp_map_flags {
    spm_READ    = 1 << 0,
    spm_WRITE   = 1 << 1,
    spm_PRIVATE = 1 << 2,
    spm_SHARED  = 1 << 3,
} sp_map_flags;

//NOTE(ELI): To create and write a file of a specific length,
//just modify the metadata to the correct length
void* sp_MemMapFile(sp_file f, spf_metadata meta, sp_map_flags flags);
void sp_UnMapFile(void* data, spf_metadata meta);

void* sp_PageAlloc(uint64_t size);
void  sp_PageFree(void* p);

//On windows this will open a terminal if one does not
//already exist, on Linux this will just redirect
//to normal printf
void sp_Printf(const char* format, ...);

//This takes in the raw file descriptor and writes to it
//The main difference is that on windows it would be a
//void* HANDLE
void sp_fPrintf(sp_file f, const char* format, ...);


void* sp_HeapAlloc(uint64_t size);
void* sp_HeapRealloc(void* p, uint64_t size);

//NOTE(ELI): I really think the standard Calloc API
//is silly, so I just made it have the same api as
//Malloc, which is what it always should have been
void* sp_HeapCalloc(uint64_t size);
void sp_HeapFree(void* p);


void sp_Chdir(const char* dir);
void sp_Mkdir(const char* dir);


#endif
