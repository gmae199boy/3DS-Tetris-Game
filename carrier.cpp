#include "carrier.h"
#include "block.h"
#include "wall.h"
#include <string.h>

#define CARRIER_BUFFER_MAX (4*4)

static const char BLOCK_PATTERN_DATA[][CARRIER_BUFFER_MAX] = {

	{ // BLOCK_PATTERN_I
		9, 9, 9, 9,
		0, 0, 0, 0,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
	{ // BLOCK_PATTERN_O
		9, 1, 1, 9,
		9, 1, 1, 9,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
	{ // BLOCK_PATTERN_S
		9, 2, 2, 9,
		2, 2, 9, 9,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
	{ // BLOCK_PATTERN_Z
		3, 3, 9, 9,
		9, 3, 3, 9,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
	{ // BLOCK_PATTERN_J
		4, 9, 9, 9,
		4, 4, 4, 9,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
	{ // BLOCK_PATTERN_L
		9, 9, 5, 9,
		5, 5, 5, 9,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
	{ // BLOCK_PATTERN_T
		9, 6, 9, 9,
		6, 6, 6, 9,
		9, 9, 9, 9,
		9, 9, 9, 9,
	},
};

// キャリア
static char g_Carrier[CARRIER_BUFFER_MAX];
// キャリアの現在のブロック座標x
static int g_CarrierBX = 0;
// キャリアの現在のブロック座標y
static int g_CarrierBY = 0;


/* ローカル関数のプロトタイプ宣言 */

// キャリアから壁にブロックを転送できるか調べる
//
// 戻り値:転送できるときはtrue
//
static bool canCarrierToWall(void);

// キャリアから壁にブロックを転送する
//
// 戻り値:転送できなかったときはfalse
//
static bool carrierToWall(void);

// キャリアからブロックを取得する
static BLOCK_TYPE getBlock(int bx, int by);

// キャリアのパターンを壁から消す
static void wallClearFromCarrierPattern(void);


// キャリアを先頭に戻して指定のブロックを配置する
//
// 引数:pattern ... ブロックパターン
//
// 戻り値:無事置けたらtrue
//
bool Carrier_Reset(BLOCK_PATTERN pattern)
{
	// キャリアの座標をリセットする
	g_CarrierBX = 3;
	g_CarrierBY = 0;

	// 指定のパターンをキャリアにコピーする
	for (int i = 0; i < CARRIER_BUFFER_MAX; i++) {
		g_Carrier[i] = BLOCK_PATTERN_DATA[pattern][i];
	}
	// 転送先アドレス, 転送元アドレス, 転送サイズ
	// memcpy(g_Carrier, BLOCK_PATTERN_DATA[pattern], CARRIER_BUFFER_MAX);

	// キャリアのパターンを壁に転送する
	return carrierToWall();
}

// キャリアの更新
void Carrier_Update(void)
{
	wallClearFromCarrierPattern();
	g_CarrierBY++;

	if( !carrierToWall() ) {
		g_CarrierBY--;
		carrierToWall();
	}
}

// キャリアの移動
void Carrier_Down(void)
{

}

void Carrier_Left(void)
{

}

void Carrier_Right(void)
{

}

// キャリアの回転(時計回り)
void Carrier_RotationCW(void)
{

}

// キャリアの回転(逆時計回り)
void Carrier_RotationCCW(void)
{

}

// キャリアが落ちてるかどうか
//
// 戻り値:更新したとき、落ちていたらtrue
//
bool Carrier_IsFall(void)
{

}

// キャリアから壁にブロックを転送できるか調べる
//
// 戻り値:転送できるときはtrue
//
static bool canCarrierToWall(void)
{
	for (int by = 0; by < 4; by++) {
		for (int bx = 0; bx < 4; bx++) {
			
			// キャリアの空の部分は無視
			if (getBlock(bx, by) == BLOCK_TYPE_NONE) {
				continue;
			}

			// 壁の外や壁がすでにブロックで埋まっていた場合
			if (Wall_GetBlock(g_CarrierBX + bx, g_CarrierBY + by) != BLOCK_TYPE_NONE) {
				return false;
			}
		}
	}

	return true;
}

// キャリアから壁にブロックを転送する
//
// 戻り値:転送できなかったときはfalse
//
static bool carrierToWall(void)
{
	// キャリアから壁にブロックを転送できるか？
	if (!canCarrierToWall()) {
		// できんかった！！
		return false;
	}

	// キャリアから壁にブロックを転送するのに邪魔なもの無し！！

	for (int by = 0; by < 4; by++) {
		for (int bx = 0; bx < 4; bx++) {

			BLOCK_TYPE block_type = getBlock(bx, by);

			// キャリアの空の部分は無視
			if (block_type == BLOCK_TYPE_NONE) {
				continue;
			}

			// 壁にブロックを配置
			Wall_SetBlock(block_type, g_CarrierBX + bx, g_CarrierBY + by);
		}
	}
}

// キャリアからブロックを取得する
static BLOCK_TYPE getBlock(int bx, int by)
{
	// 2次元 -> １次元
	// 横(x) + 横の数(w) * 縦(y)
	return (BLOCK_TYPE)g_Carrier[bx + 4 * by];
}

// キャリアのパターンを壁から消す
static void wallClearFromCarrierPattern(void)
{
	for (int by = 0; by < 4; by++) {
		for (int bx = 0; bx < 4; bx++) {

			BLOCK_TYPE block_type = getBlock(bx, by);

			// キャリアの空の部分は無視
			if (block_type == BLOCK_TYPE_NONE) {
				continue;
			}

			// 壁にブロックを配置
			Wall_SetBlock(BLOCK_TYPE_NONE, 
				g_CarrierBX + bx, g_CarrierBY + by);
		}
	}
}
