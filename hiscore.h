#ifndef HISCORE_H_
#define HISCORE_H_

#include <nn/hid.h>

void Hiscore_Initialize(void);
void Hiscore_Finalize(void);
void Hiscore_Update(nn::hid::PadStatus* pPadStatus);
void Hiscore_Draw0(void);
void Hiscore_Draw1(void);

#endif // HISCORE_H_
