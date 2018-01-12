#ifndef FAMITONE2_H_
#define FAMITONE2_H_

#include <stdbool.h>
#include <stdint.h>

void __fastcall__ FamiToneInit(void);

void __fastcall__ FamiToneSfxInit(void);

void __fastcall__ FamiToneMusicPlay(uint8_t song);

void __fastcall__ FamiToneSfxPlay(uint8_t effect);

void FamiToneUpdate(void);

#endif // FAMITONE2_H_

