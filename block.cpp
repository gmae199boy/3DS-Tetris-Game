#include "block.h"
#include "Polygon.h"


static int g_TextureNum = -1;


// �u���b�N���W���[���̏�����
//
// ����:texNum ... �u���b�N�̕`��ɕK�v�ȃe�N�X�`���ԍ�
//
void Block_Initialize(int texNum)
{
	g_TextureNum = texNum;
}

// �u���b�N�̕`��
//
// ����:type ... �u���b�N�̎��
//      x    ... �u���b�N�`����Wx
//      y    ... �u���b�N�`����Wy
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
		20 * type, // �؂�����Wx
		20,
		20,
		20
		);
}
