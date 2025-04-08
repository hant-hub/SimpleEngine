#include "tokenizer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


StructMember ParseMember(Tokenizer* t) {
    StructMember m = {0};

    //consume until semicolon
    Token type = GetToken(t, 1);
    m.t.isStruct = 0;
    if (type.t == TOKEN_STRUCT) {
        m.t.isStruct = 1;
        type = GetToken(t, 0);
    }

    Token pointer = GetToken(t, 0);

    m.t.basename = type.start;
    m.t.nameSize = type.size;
    m.t.isPointer = pointer.t == '*';

    Token s = GetToken(t, 1);
    //printf("s: %.*s\n", s.size, s.start);
    while (s.t != ';' && s.t != '}' && s.t != TOKEN_EOF) {
        //printf("eaten: %d %.*s\n", s.t, s.size, s.start);
        s = GetToken(t, 0);
    }

    s = GetToken(t, 2);
    m.name = s.start;
    m.nameSize = s.size;

    //printf("%.*s\n", m.nameSize, m.name);

    return m;
}

StructData* ParseStruct(Tokenizer* t, StructData* head) {
    Token name = GetToken(t, 0);
    if (name.t == TOKEN_EOF) return head;

    uint32_t cap = 5;
    uint32_t top = 0;
    StructMember* members = malloc(sizeof(StructMember) * cap);

    //skip opening brace
    if (name.t != '{') GetToken(t, 0);
    Token m = GetToken(t, 0);
    while (m.t != '}' && m.t != TOKEN_EOF) {
        //printf("m: %.*s\n", m.size, m.start);
        if (top + 1 > cap) {
            cap *= 2;
            members = (StructMember*)realloc(members, sizeof(StructMember) * cap);
        }
        members[top++] = ParseMember(t);
        m = GetToken(t, 0);

        //nested struct
        if (m.t == TOKEN_STRUCT) {
            head = ParseStruct(t, head);
            
            if (top + 1 > cap) {
                cap *= 2;
                members = (StructMember*)realloc(members, sizeof(StructMember) * cap);
            }
            
            Token memname = GetToken(t, 0);
            members[top++] = (StructMember){
                .name = memname.start,
                .nameSize = memname.size,
                .t = {
                    .basename = head->name,
                    .nameSize = head->nameSize,
                    .isPointer = 0,
                    .isStruct = 1,
                }
            };
        }
    }

    StructData* new = malloc(sizeof(StructData));
    new->name = name.start;
    new->nameSize = name.size;
    new->members = members;
    new->numMembers = top;
    new->next = head;

    return new;
}


StructData* GetStructData(File* f) {
    StructData* head = NULL;

    int fd = f->fd;
    struct stat s;
    fstat(fd, &s);

    char* data = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    printf("size: %ld\n", s.st_size);

    f->size = s.st_size;
    f->data = data;

    Token t;
    Tokenizer tok = {
        .data = data,
        .At = data,
        .size = (uint32_t)s.st_size
    };

    while ((t = GetToken(&tok, 0)).t != TOKEN_EOF) {
        if (t.t == TOKEN_STRUCT) {
            head = ParseStruct(&tok, head);
        }
    }

    return head;
}



void FreeStructData(StructData* head, File f) {
    StructData* prev = head;
    StructData* curr = head->next;
    while (curr) {
        free(prev->members);
        free(prev);

        prev = curr;
        curr = curr->next;
    }

    munmap(f.data, f.size);
    close(f.fd);
}
