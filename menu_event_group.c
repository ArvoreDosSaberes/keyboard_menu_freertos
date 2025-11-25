#include "FreeRTOS.h"
#include "event_groups.h"
#include "stdio.h"
#include <stdbool.h>

#include "oled.h"
#include "log.h"

#include "menu_event_group.h"

#ifdef __cplusplus
extern "C" {
#endif

EventGroupHandle_t xMenuEventGroup;

bool create_menu_event_group() {
    xMenuEventGroup = xEventGroupCreate();
    if (xMenuEventGroup == NULL) {
        LOG_WARN("[FreeRTOS] Erro ao criar grupo de eventos para o menu");
        return false;
    }
    LOG_INFO("[FreeRTOS] Grupo de eventos para o menu criados");
    return true;
}   

#ifdef __cplusplus
}
#endif // __cplusplus
