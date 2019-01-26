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

// �L�����A
static char g_Carrier[CARRIER_BUFFER_MAX];
// �L�����A�̌��݂̃u���b�N���Wx
static int g_CarrierBX = 0;
// �L�����A�̌��݂̃u���b�N���Wy
static int g_CarrierBY = 0;


/* ���[�J���֐��̃v���g�^�C�v�錾 */

// �L�����A����ǂɃu���b�N��]���ł��邩���ׂ�
//
// �߂�l:�]���ł���Ƃ���true
//
static bool canCarrierToWall(void);

// �L�����A����ǂɃu���b�N��]������
//
// �߂�l:�]���ł��Ȃ������Ƃ���false
//
static bool carrierToWall(void);

// �L�����A����u���b�N���擾����
static BLOCK_TYPE getBlock(int bx, int by);

// �L�����A�̃p�^�[����ǂ������
static void wallClearFromCarrierPattern(void);


// �L�����A��擪�ɖ߂��Ďw��̃u���b�N��z�u����
//
// ����:pattern ... �u���b�N�p�^�[��
//
// �߂�l:�����u������true
//
bool Carrier_Reset(BLOCK_PATTERN pattern)
{
	// �L�����A�̍��W�����Z�b�g����
	g_CarrierBX = 3;
	g_CarrierBY = 0;

	// �w��̃p�^�[�����L�����A�ɃR�s�[����
	for (int i = 0; i < CARRIER_BUFFER_MAX; i++) {
		g_Carrier[i] = BLOCK_PATTERN_DATA[pattern][i];
	}
	// �]����A�h���X, �]�����A�h���X, �]���T�C�Y
	// memcpy(g_Carrier, BLOCK_PATTERN_DATA[pattern], CARRIER_BUFFER_MAX);

	// �L�����A�̃p�^�[����ǂɓ]������
	return carrierToWall();
}

// �L�����A�̍X�V
void Carrier_Update(void)
{
	wallClearFromCarrierPattern();
	g_CarrierBY++;

	if( !carrierToWall() ) {
		g_CarrierBY--;
		carrierToWall();
	}
}

// �L�����A�̈ړ�
void Carrier_Down(void)
{

}

void Carrier_Left(void)
{

}

void Carrier_Right(void)
{

}

// �L�����A�̉�](���v���)
void Carrier_RotationCW(void)
{

}

// �L�����A�̉�](�t���v���)
void Carrier_RotationCCW(void)
{

}

// �L�����A�������Ă邩�ǂ���
//
// �߂�l:�X�V�����Ƃ��A�����Ă�����true
//
bool Carrier_IsFall(void)
{

}

// �L�����A����ǂɃu���b�N��]���ł��邩���ׂ�
//
// �߂�l:�]���ł���Ƃ���true
//
static bool canCarrierToWall(void)
{
	for (int by = 0; by < 4; by++) {
		for (int bx = 0; bx < 4; bx++) {
			
			// �L�����A�̋�̕����͖���
			if (getBlock(bx, by) == BLOCK_TYPE_NONE) {
				continue;
			}

			// �ǂ̊O��ǂ����łɃu���b�N�Ŗ��܂��Ă����ꍇ
			if (Wall_GetBlock(g_CarrierBX + bx, g_CarrierBY + by) != BLOCK_TYPE_NONE) {
				return false;
			}
		}
	}

	return true;
}

// �L�����A����ǂɃu���b�N��]������
//
// �߂�l:�]���ł��Ȃ������Ƃ���false
//
static bool carrierToWall(void)
{
	// �L�����A����ǂɃu���b�N��]���ł��邩�H
	if (!canCarrierToWall()) {
		// �ł��񂩂����I�I
		return false;
	}

	// �L�����A����ǂɃu���b�N��]������̂Ɏז��Ȃ��̖����I�I

	for (int by = 0; by < 4; by++) {
		for (int bx = 0; bx < 4; bx++) {

			BLOCK_TYPE block_type = getBlock(bx, by);

			// �L�����A�̋�̕����͖���
			if (block_type == BLOCK_TYPE_NONE) {
				continue;
			}

			// �ǂɃu���b�N��z�u
			Wall_SetBlock(block_type, g_CarrierBX + bx, g_CarrierBY + by);
		}
	}
}

// �L�����A����u���b�N���擾����
static BLOCK_TYPE getBlock(int bx, int by)
{
	// 2���� -> �P����
	// ��(x) + ���̐�(w) * �c(y)
	return (BLOCK_TYPE)g_Carrier[bx + 4 * by];
}

// �L�����A�̃p�^�[����ǂ������
static void wallClearFromCarrierPattern(void)
{
	for (int by = 0; by < 4; by++) {
		for (int bx = 0; bx < 4; bx++) {

			BLOCK_TYPE block_type = getBlock(bx, by);

			// �L�����A�̋�̕����͖���
			if (block_type == BLOCK_TYPE_NONE) {
				continue;
			}

			// �ǂɃu���b�N��z�u
			Wall_SetBlock(BLOCK_TYPE_NONE, 
				g_CarrierBX + bx, g_CarrierBY + by);
		}
	}
}
