#ifndef CARRIER_H_
#define CARRIER_H_


// ブロックパターン列挙型
typedef enum
{
	BLOCK_PATTERN_I,
	BLOCK_PATTERN_O,
	BLOCK_PATTERN_S,
	BLOCK_PATTERN_Z,
	BLOCK_PATTERN_J,
	BLOCK_PATTERN_L,
	BLOCK_PATTERN_T,
	BLOCK_PATTERN_MAX
}BLOCK_PATTERN;


// キャリアを先頭に戻して指定のブロックを配置する
//
// 引数:pattern ... ブロックパターン
//
// 戻り値:無事置けたらtrue
//
bool Carrier_Reset(BLOCK_PATTERN pattern);

// キャリアの更新
void Carrier_Update(void);

// キャリアの移動
void Carrier_Down(void);
void Carrier_Left(void);
void Carrier_Right(void);

// キャリアの回転(時計回り)
void Carrier_RotationCW(void);

// キャリアの回転(逆時計回り)
void Carrier_RotationCCW(void);

// キャリアが落ちてるかどうか
//
// 戻り値:更新したとき、落ちていたらtrue
//
bool Carrier_IsFall(void);


#endif // CARRIER_H_
