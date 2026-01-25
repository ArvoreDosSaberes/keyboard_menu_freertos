#ifndef MENU_OLED_TASK_H
#define MENU_OLED_TASK_H

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "rack_inteligente_parametros.h"

#ifdef __cplusplus
extern "C" {
#endif

void vMenuOledTask(void *pvParameters);

bool menuOledIsOpen(void);

#ifdef __cplusplus
}
#endif

#endif
