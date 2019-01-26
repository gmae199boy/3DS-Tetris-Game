#include "wall.h"
#include "block.h"


#define WALL_BLOCK_COUNT_MAX    (WALL_BLOCK_H_MAX * WALL_BLOCK_V_MAX)
#define WALL_BLOCK_V_TOP_MAX    (9)
#define WALL_BLOCK_V_BOTTOM_MAX (WALL_BLOCK_V_MAX - WALL_BLOCK_V_TOP_MAX)
#define WALL_OFFSET_TOP_X       (60.0f)
#define WALL_OFFSET_TOP_Y       (60.0f)
#define WALL_OFFSET_BOTTOM_X    (20.0f)
#define WALL_OFFSET_BOTTOM_Y    ( 0.0f)


static char g_WallData[WALL_BLOCK_COUNT_MAX] = {
	1, 2, 9, 9, 9, 9, 9, 9, 3, 4,
	2, 1, 9, 9, 9, 9, 9, 9, 4, 3,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	3, 4, 9, 9, 9, 9, 9, 9, 5, 6,
	4, 3, 9, 9, 9, 9, 9, 9, 6, 5,
	
	1, 1, 9, 9, 9, 9, 9, 9, 0, 0,
	1, 1, 9, 9, 9, 9, 9, 9, 0, 0,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	4, 5, 9, 9, 9, 9, 9, 9, 6, 6,
	5, 4, 9, 9, 9, 9, 9, 9, 6, 6,
};


void Wall_Initialize(void)
{
	// g_WallDataを初期化(全部9にする)
	for (int i = 0; i < WALL_BLOCK_COUNT_MAX; i++) {
		g_WallData[i] = (char)BLOCK_TYPE_NONE;
	}
}

void Wall_DrawTop(void)
{
	for (int j = 0; j < WALL_BLOCK_V_TOP_MAX; j++) {
		for (int i = 0; i < WALL_BLOCK_H_MAX; i++) {

			int index = j * WALL_BLOCK_H_MAX + i;
			float dx = WALL_OFFSET_TOP_X + BLOCK_SIZE * i;
			float dy = WALL_OFFSET_TOP_Y + BLOCK_SIZE * j;

			Block_Draw((BLOCK_TYPE)g_WallData[index], dx, dy);
		}
	}
}

void Wall_DrawBottom(void)
{
	for (int j = 0; j < WALL_BLOCK_V_BOTTOM_MAX; j++) {
		for (int i = 0; i < WALL_BLOCK_H_MAX; i++) {

			int index = (WALL_BLOCK_V_TOP_MAX + j) * WALL_BLOCK_H_MAX + i;
			float dx = WALL_OFFSET_BOTTOM_X + BLOCK_SIZE * i;
			float dy = WALL_OFFSET_BOTTOM_Y + BLOCK_SIZE * j;

			Block_Draw((BLOCK_TYPE)g_WallData[index], dx, dy);
		}
	}
}

void Wall_SetBlock(BLOCK_TYPE type, int bx, int by)
{
	// もしbx, byが範囲外ならば、代入しない
	if (bx < 0 || bx >= WALL_BLOCK_H_MAX) return;
	if (by < 0 || by >= WALL_BLOCK_V_MAX) return;

	// g_WallDataのbx, byの場所にtypeを代入する
	g_WallData[bx + WALL_BLOCK_H_MAX * by] = (char)type;
}

BLOCK_TYPE Wall_GetBlock(int bx, int by)
{
	// bx, byが範囲外ならばBLOCK_TYPE_WALLOUTをreturnする
	if (bx < 0 || bx >= WALL_BLOCK_H_MAX) return BLOCK_TYPE_WALLOUT;
	if (by < 0 || by >= WALL_BLOCK_V_MAX) return BLOCK_TYPE_WALLOUT;

	// g_WallDataのbx, byの場所のtypeをreturnする
	return (BLOCK_TYPE)g_WallData[bx + WALL_BLOCK_H_MAX * by];
}
