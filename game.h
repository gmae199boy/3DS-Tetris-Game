#ifndef GAME_H_
#define GAME_H_

#include <nn/hid.h>

void Game_Initialize(void);
void Game_Finalize(void);
void Game_Update(nn::hid::PadStatus* pPadStatus);
void Game_Draw0(void);
void Game_Draw1(void);

#endif // GAME_H_
