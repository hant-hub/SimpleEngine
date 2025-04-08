#ifndef GENERATE_H
#define GENERATE_H
#include "parser.h"

#include <stdio.h>

void GenerateStructDefHeader(const char* name, StructData* head);
void GenerateStructDefSource(const char* name, StructData* head);


#endif
