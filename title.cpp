#include <nn/hid.h>
#include "scene.h"
#include "textureManager.h" // ①
#include "Polygon.h"
#include "fade.h"


static int g_TexNum = -1; // ②
static bool g_bTitleEnd = false;


void Title_Initialize(void)
{
	// ③
	g_TexNum = TextureManager_SetFile("rom:/title.tga", 512, 512);
	TextureManager_Load(); // ④ 
	g_bTitleEnd = false;
}

void Title_Finalize(void)
{
	TextureManager_Release(g_TexNum);
}

void Title_Update(nn::hid::PadStatus* pPadStatus)
{
	if (pPadStatus->trigger & nn::hid::BUTTON_A) {
		Fade_Start(0.0f, 0.0f, 0.0f, 60, true);
		g_bTitleEnd = true;
	}

	if (g_bTitleEnd && !Fade_IsFade()) {
		Scene_ChangeScene(SCENE_GAME);
	}
}

void Title_Draw0(void)
{
	//              ⑤　　　　　　　　　　　　　　　　　　　　　　⑥
	Polygon_Draw(g_TexNum, 0.0f, 0.0f, 400, 240, 0, 0, 400, 240);
}

void Title_Draw1(void)
{

}
