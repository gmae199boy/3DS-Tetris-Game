#ifndef FADE_H_
#define FADE_H_

void Fade_Initialize(void);
void Fade_Finalize(void);
void Fade_Update(void);
void Fade_Draw(void);

// �t�F�[�h�̃X�^�[�g
//
// ����:r, g, b ... �t�F�[�h�J���[�̐ݒ�
//      frame   ... �t�F�[�h����(�t���[����)
//      bOut    ... �t�F�[�h�C����������false
//                  �t�F�[�h�A�E�g��������true
//
void Fade_Start(float r, float g, float b, int frame, bool bOut);

// �t�F�[�h�����H
//
// �߂�l:�t�F�[�h����������true
//
bool Fade_IsFade(void);

#endif // FADE_H_

