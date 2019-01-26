#ifndef WALL_H_
#define WALL_H_


#include "block.h"


#define WALL_BLOCK_H_MAX (10)
#define WALL_BLOCK_V_MAX (20)

void Wall_Initialize(void);
void Wall_DrawTop(void);
void Wall_DrawBottom(void);

void Wall_SetBlock(BLOCK_TYPE type, int bx, int by);
BLOCK_TYPE Wall_GetBlock(int bx, int by);

#endif // WALL_H_
