#include "block.h"
#include "Polygon.h"


static int g_TextureNum = -1;


// ブロックモジュールの初期化
//
// 引数:texNum ... ブロックの描画に必要なテクスチャ番号
//
void Block_Initialize(int texNum)
{
	g_TextureNum = texNum;
}

// ブロックの描画
//
// 引数:type ... ブロックの種類
//      x    ... ブロック描画座標x
//      y    ... ブロック描画座標y
//
void Block_Draw(BLOCK_TYPE type, float x, float y)
{
	if (type == BLOCK_TYPE_NONE || type == BLOCK_TYPE_WALLOUT) {
		return;
	}

	Polygon_Draw(
		g_TextureNum,
		x,
		y,
		20,
		20,
		20 * type, // 切り取り座標x
		20,
		20,
		20
		);
}
