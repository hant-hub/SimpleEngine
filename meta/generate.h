#ifndef GENERATE_H
#define GENERATE_H
#include "parser.h"

#include <stdio.h>

void GenerateStructDefHeader(FILE* out, StructData* head);
void GenerateStructDefSource(FILE* out, StructData* head);


#endif
