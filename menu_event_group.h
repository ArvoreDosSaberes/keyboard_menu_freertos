#ifndef MENU_EVENT_GROUP_H
#define MENU_EVENT_GROUP_H

#include "FreeRTOS.h"
#include "event_groups.h"
#include "stdio.h"
#include <stdbool.h>

#include "oled.h"
#include "log_vt100.h"

#ifdef __cplusplus
extern "C" {
#endif

bool create_menu_event_group();

// Lançado sempre que uma tecla é precionada (ainda dentro do limite de 24 bits)
#define xKeyboardBitsToWaitFor                (1 << 5)

// Cada tecla usa um bit exclusivo, todos abaixo de 24 bits
#define xKeyboard_Key_1_BitsToWaitFor         (1 << 6)
#define xKeyboard_Key_2_BitsToWaitFor         (1 << 7)
#define xKeyboard_Key_3_BitsToWaitFor         (1 << 8)
#define xKeyboard_Key_4_BitsToWaitFor         (1 << 9)
#define xKeyboard_Key_5_BitsToWaitFor         (1 << 10)
#define xKeyboard_Key_6_BitsToWaitFor         (1 << 11)
#define xKeyboard_Key_7_BitsToWaitFor         (1 << 12)
#define xKeyboard_Key_8_BitsToWaitFor         (1 << 13)
#define xKeyboard_Key_9_BitsToWaitFor         (1 << 14)
#define xKeyBoard_Key_A_BitsToWaitFor         (1 << 15)
#define xKeyBoard_Key_B_BitsToWaitFor         (1 << 16)
#define xKeyBoard_Key_C_BitsToWaitFor         (1 << 17)
#define xKeyBoard_Key_D_BitsToWaitFor         (1 << 18)
#define xKeyBoard_Key_E_BitsToWaitFor         (1 << 19)
#define xKeyBoard_Key_F_BitsToWaitFor         (1 << 20)
#define xKeyBoard_Key_sharp_BitsToWaitFor     (1 << 21)
#define xKeyBoard_Key_asterisco_BitsToWaitFor (1 << 22)


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MENU_EVENT_GROUP_H