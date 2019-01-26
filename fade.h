#ifndef FADE_H_
#define FADE_H_

void Fade_Initialize(void);
void Fade_Finalize(void);
void Fade_Update(void);
void Fade_Draw(void);

// フェードのスタート
//
// 引数:r, g, b ... フェードカラーの設定
//      frame   ... フェード時間(フレーム数)
//      bOut    ... フェードインだったらfalse
//                  フェードアウトだったらtrue
//
void Fade_Start(float r, float g, float b, int frame, bool bOut);

// フェード中か？
//
// 戻り値:フェード中だったらtrue
//
bool Fade_IsFade(void);

#endif // FADE_H_

