#ifndef BLOCK_H_
#define BLOCK_H_


#define BLOCK_SIZE (20)


// �u���b�N���W���[���̏�����
//
// ����:texNum ... �u���b�N�̕`��ɕK�v�ȃe�N�X�`���ԍ�
//
void Block_Initialize(int texNum);

// �u���b�N�̎�ޗ񋓌^
typedef enum
{
	BLOCK_TYPE_WALLOUT = -1, // �͈͊O
	BLOCK_TYPE_SKYBLUE,
	BLOCK_TYPE_YELLOW,
	BLOCK_TYPE_LIGHTGREEN,
	BLOCK_TYPE_RED,
	BLOCK_TYPE_BLUE,
	BLOCK_TYPE_ORANGE,
	BLOCK_TYPE_PURPLE,
	BLOCK_TYPE_COLOR_COUNT, // �F��
	BLOCK_TYPE_NONE = 9
}BLOCK_TYPE;

// �u���b�N�̕`��
//
// ����:type ... �u���b�N�̎��
//      x    ... �u���b�N�`����Wx
//      y    ... �u���b�N�`����Wy
//
void Block_Draw(BLOCK_TYPE type, float x, float y);


#endif // BLOCK_H_
