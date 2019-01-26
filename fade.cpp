#include "Polygon.h"
#include "textureManager.h"
#include "main.h"


static float g_FadeR = 1.0f;
static float g_FadeG = 1.0f;
static float g_FadeB = 1.0f;
static int g_FadeFrame = 0;
static int g_FadeCount = 0;
static bool g_bFadeOut = false;
static bool g_bFade = false; // �t�F�[�h�����H

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

	// �e�N�X�`���̓ǂݍ���
	g_FadeTexture = TextureManager_SetFile("rom:/white.tga", 8, 8);
	TextureManager_Load();
}

void Fade_Finalize(void)
{
	// �e�N�X�`���̉��
	TextureManager_Release(g_FadeTexture);
}

void Fade_Update(void)
{
	if (!g_bFade) {
		return;
	}

	// �t�F�[�h���`
	g_FadeCount++;

	// �I������
	if (g_FadeCount > g_FadeFrame) {
		g_FadeCount = g_FadeFrame;
		g_bFade = false; // �t�F�[�h�I��
	}
}

void Fade_Draw(void)
{
	if (!g_bFade && !g_bFadeOut) {
		return;
	}

	// ���l�͂����H
	float a = (float)g_FadeCount / g_FadeFrame;
	
	// �t�F�[�h�C���������烿�l�͋t
	if (!g_bFadeOut) {
		a = 1.0f - a;
	}

	// �t�F�[�h�J���[�ƃ��l��ݒ�
	Polygon_SetColor(g_FadeR, g_FadeG, g_FadeB, a);

	// ��ʃT�C�Y�̃|���S����`��
	Polygon_Draw(g_FadeTexture, 0.0f, 0.0f, 
		SCREEN_0_WIDTH, SCREEN_0_HEIGHT);

	// �|���S����`���I������猳�ɖ߂�
	Polygon_SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// �t�F�[�h�̃X�^�[�g
//
// ����:r, g, b ... �t�F�[�h�J���[�̐ݒ�
//      frame   ... �t�F�[�h����(�t���[����)
//      bOut    ... �t�F�[�h�C����������false
//                  �t�F�[�h�A�E�g��������true
//
void Fade_Start(float r, float g, float b, int frame, bool bOut)
{
	g_FadeR = r;
	g_FadeG = g;
	g_FadeB = b;

	g_FadeFrame = frame;
	g_FadeCount = 0;
	g_bFadeOut = bOut;

	g_bFade = true; // �t�F�[�h�X�^�[�g
}

// �t�F�[�h�����H
//
// �߂�l:�t�F�[�h����������true
//
bool Fade_IsFade(void)
{
	return g_bFade;
}
