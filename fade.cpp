#include "Polygon.h"
#include "textureManager.h"
#include "main.h"


static float g_FadeR = 1.0f;
static float g_FadeG = 1.0f;
static float g_FadeB = 1.0f;
static int g_FadeFrame = 0;
static int g_FadeCount = 0;
static bool g_bFadeOut = false;
static bool g_bFade = false; // フェード中か？

static int g_FadeTexture = -1;


void Fade_Initialize(void)
{
	g_FadeR = 1.0f;
	g_FadeG = 1.0f;
	g_FadeB = 1.0f;
	g_FadeFrame = 0;
	g_FadeCount = 0;
	g_bFadeOut = false;
	g_bFade = false;

	// テクスチャの読み込み
	g_FadeTexture = TextureManager_SetFile("rom:/white.tga", 8, 8);
	TextureManager_Load();
}

void Fade_Finalize(void)
{
	// テクスチャの解放
	TextureManager_Release(g_FadeTexture);
}

void Fade_Update(void)
{
	if (!g_bFade) {
		return;
	}

	// フェード中～
	g_FadeCount++;

	// 終了判定
	if (g_FadeCount > g_FadeFrame) {
		g_FadeCount = g_FadeFrame;
		g_bFade = false; // フェード終了
	}
}

void Fade_Draw(void)
{
	if (!g_bFade && !g_bFadeOut) {
		return;
	}

	// α値はいくつ？
	float a = (float)g_FadeCount / g_FadeFrame;
	
	// フェードインだったらα値は逆
	if (!g_bFadeOut) {
		a = 1.0f - a;
	}

	// フェードカラーとα値を設定
	Polygon_SetColor(g_FadeR, g_FadeG, g_FadeB, a);

	// 画面サイズのポリゴンを描く
	Polygon_Draw(g_FadeTexture, 0.0f, 0.0f, 
		SCREEN_0_WIDTH, SCREEN_0_HEIGHT);

	// ポリゴンを描き終わったら元に戻す
	Polygon_SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// フェードのスタート
//
// 引数:r, g, b ... フェードカラーの設定
//      frame   ... フェード時間(フレーム数)
//      bOut    ... フェードインだったらfalse
//                  フェードアウトだったらtrue
//
void Fade_Start(float r, float g, float b, int frame, bool bOut)
{
	g_FadeR = r;
	g_FadeG = g;
	g_FadeB = b;

	g_FadeFrame = frame;
	g_FadeCount = 0;
	g_bFadeOut = bOut;

	g_bFade = true; // フェードスタート
}

// フェード中か？
//
// 戻り値:フェード中だったらtrue
//
bool Fade_IsFade(void)
{
	return g_bFade;
}
