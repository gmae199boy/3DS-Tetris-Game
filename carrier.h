#ifndef CARRIER_H_
#define CARRIER_H_


// �u���b�N�p�^�[���񋓌^
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


// �L�����A��擪�ɖ߂��Ďw��̃u���b�N��z�u����
//
// ����:pattern ... �u���b�N�p�^�[��
//
// �߂�l:�����u������true
//
bool Carrier_Reset(BLOCK_PATTERN pattern);

// �L�����A�̍X�V
void Carrier_Update(void);

// �L�����A�̈ړ�
void Carrier_Down(void);
void Carrier_Left(void);
void Carrier_Right(void);

// �L�����A�̉�](���v���)
void Carrier_RotationCW(void);

// �L�����A�̉�](�t���v���)
void Carrier_RotationCCW(void);

// �L�����A�������Ă邩�ǂ���
//
// �߂�l:�X�V�����Ƃ��A�����Ă�����true
//
bool Carrier_IsFall(void);


#endif // CARRIER_H_
