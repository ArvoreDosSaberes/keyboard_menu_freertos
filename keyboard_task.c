#include <hardware/timer.h>
#include <stdint.h>

#include "keyboard_menu_parameters.h"
#include "menu_event_group.h"
#include "keyboard.h"

#include "FreeRTOS.h"
#include "task.h"

#include "log.h"

extern EventGroupHandle_t xMenuEventGroup;

static EventBits_t keyboard_get_bits_for_key(char key);

void keyboard_task(__attribute__((unused)) void *pvParameters) {
  LOG_INFO("[Keyboard] Iniciando...");
  keyboard_init();

  while (1) {
    char k = keyboard_scan();
    if (k != 0) {
      LOG_INFO("[Keyboard] Key pressed: %c", k);

      EventBits_t bits = keyboard_get_bits_for_key(k);
      if (bits != 0) {
        xEventGroupSetBits(xMenuEventGroup, bits);
      }

    }

    vTaskDelay(pdMS_TO_TICKS(KBD_TASK_DELAY));
  }
}

static EventBits_t keyboard_get_bits_for_key(char key) {
  EventBits_t bits = xKeyboardBitsToWaitFor;

  switch (key) {
  case '1':
    bits |= xKeyboard_Key_1_BitsToWaitFor;
    break;
  case '2':
    bits |= xKeyboard_Key_2_BitsToWaitFor;
    break;
  case '3':
    bits |= xKeyboard_Key_3_BitsToWaitFor;
    break;
  case '4':
    bits |= xKeyboard_Key_4_BitsToWaitFor;
    break;
  case '5':
    bits |= xKeyboard_Key_5_BitsToWaitFor;
    break;
  case '6':
    bits |= xKeyboard_Key_6_BitsToWaitFor;
    break;
  case '7':
    bits |= xKeyboard_Key_7_BitsToWaitFor;
    break;
  case '8':
    bits |= xKeyboard_Key_8_BitsToWaitFor;
    break;
  case '9':
    bits |= xKeyboard_Key_9_BitsToWaitFor;
    break;
  case 'A':
    bits |= xKeyBoard_Key_A_BitsToWaitFor;
    break;
  case 'B':
    bits |= xKeyBoard_Key_B_BitsToWaitFor;
    break;
  case 'C':
    bits |= xKeyBoard_Key_C_BitsToWaitFor;
    break;
  case 'D':
    bits |= xKeyBoard_Key_D_BitsToWaitFor;
    break;
  case 'E':
    bits |= xKeyBoard_Key_E_BitsToWaitFor;
    break;
  case 'F':
    bits |= xKeyBoard_Key_F_BitsToWaitFor;
    break;
  case '#':
    bits |= xKeyBoard_Key_sharp_BitsToWaitFor;
    break;
  case '*':
    bits |= xKeyBoard_Key_asterisco_BitsToWaitFor;
    break;
  default:
    break;
  }

  return bits;
}
