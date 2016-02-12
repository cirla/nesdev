/**
 * reset.h
 *
 * http://timcheeseman.com/nesdev/
 */

#ifndef RESET_H_
#define RESET_H_

extern uint8_t FrameCount;
#pragma zpsym("FrameCount");

extern uint8_t InputPort1;
#pragma zpsym("InputPort1");

extern uint8_t InputPort1Prev;
#pragma zpsym("InputPort1Prev");

extern uint8_t InputPort2;
#pragma zpsym("InputPort2");

extern uint8_t InputPort2Prev;
#pragma zpsym("InputPort2Prev");

void __fastcall__ WaitFrame(void);

#endif // RESET_H_

