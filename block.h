#ifndef BLOCK_H_
#define BLOCK_H_


#define BLOCK_SIZE (20)


// ブロックモジュールの初期化
//
// 引数:texNum ... ブロックの描画に必要なテクスチャ番号
//
void Block_Initialize(int texNum);

// ブロックの種類列挙型
typedef enum
{
	BLOCK_TYPE_WALLOUT = -1, // 範囲外
	BLOCK_TYPE_SKYBLUE,
	BLOCK_TYPE_YELLOW,
	BLOCK_TYPE_LIGHTGREEN,
	BLOCK_TYPE_RED,
	BLOCK_TYPE_BLUE,
	BLOCK_TYPE_ORANGE,
	BLOCK_TYPE_PURPLE,
	BLOCK_TYPE_COLOR_COUNT, // 色数
	BLOCK_TYPE_NONE = 9
}BLOCK_TYPE;

// ブロックの描画
//
// 引数:type ... ブロックの種類
//      x    ... ブロック描画座標x
//      y    ... ブロック描画座標y
//
void Block_Draw(BLOCK_TYPE type, float x, float y);


#endif // BLOCK_H_
