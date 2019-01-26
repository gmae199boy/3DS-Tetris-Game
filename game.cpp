#include <nn/hid.h>
#include "main.h"
#include "scene.h"
#include "fade.h"
#include "textureManager.h"
#include "bg.h"
#include "block.h"
#include "wall.h"
#include "carrier.h"


static int g_GameTextureNum = -1;


void Game_Initialize(void)
{
	// �e�N�X�`���̓ǂݍ���
	g_GameTextureNum = TextureManager_SetFile("rom:/TETRIS.tga", 512, 512);
	TextureManager_Load();

	Bg_Initialize(g_GameTextureNum);
	Block_Initialize(g_GameTextureNum);
	Wall_Initialize();

	Fade_Start(0.0f, 0.0f, 0.0f, 0, false);

	Carrier_Reset(BLOCK_PATTERN_T);
}

void Game_Finalize(void)
{
	// �e�N�X�`���̉��
	TextureManager_Release(g_GameTextureNum);
}

void Game_Update(nn::hid::PadStatus* pPadStatus)
{
	if (pPadStatus->hold & nn::hid::BUTTON_RIGHT) {
		// �E�L�[�������ꂽ
	}
	if (pPadStatus->hold & nn::hid::BUTTON_LEFT) {
		// �E�L�[�������ꂽ
	}
	if (pPadStatus->hold & nn::hid::BUTTON_UP) {
		// �E�L�[�������ꂽ
	}
	if (pPadStatus->hold & nn::hid::BUTTON_DOWN) {
		// �E�L�[�������ꂽ
	}
	
	if (pPadStatus->trigger & nn::hid::BUTTON_A) {
		Carrier_Reset(BLOCK_PATTERN_T);
	}

	// ��ʑJ�ڃe�X�g
	if (pPadStatus->trigger & nn::hid::BUTTON_START ) {
		Scene_ChangeScene(SCENE_HISCORE);
	}

	Carrier_Update();
}

void Game_Draw0(void)
{
	Bg_DrawTop();
	Wall_DrawTop();
}

void Game_Draw1(void)
{
	Bg_DrawBottom();
	Wall_DrawBottom();
}
