#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "log_vt100.h"

#include "keyboard_menu_parameters.h"
#include "menu_event_group.h"

#include "rack_inteligente.h"
#include "oled.h"
#include "oled_freeRTOS.h"
#include <hardware/gpio.h>

extern EventGroupHandle_t xMenuEventGroup;

typedef enum {
    MENU_STATE_HIDDEN = 0,
    MENU_STATE_NAVIGATE,
    MENU_STATE_EDIT_PASSWORD,
    MENU_STATE_EDIT_NAME
} menu_state_t;

static char name_edit_buffer[50];
static int name_edit_len = 0;

static char password_edit_buffer[32];
static int password_edit_len = 0;

static volatile bool menuOledOpen = false;

static void menu_oled_render(int selected_index);
static void menu_oled_render_edit(const char *title);
static void menu_oled_render_edit_value(const char *title, const char *value);

static void handle_menu_keys(EventBits_t uxBits, menu_state_t *state, int *selected_index);
static void handle_password_menu_keys(EventBits_t uxBits, menu_state_t *state, int *selected_index);
static void handle_name_menu_keys(EventBits_t uxBits, menu_state_t *state, int *selected_index);

void vMenuOledTask(void *pvParameters) {
    LOG_INFO("[Menu OLED Task] Iniciando...");
    (void) pvParameters;

    EventBits_t uxBits;
    menu_state_t state = MENU_STATE_HIDDEN;
    int selected_index = 0;

    #ifdef MENU_LED_PIN
    gpio_init(MENU_LED_PIN);
    gpio_set_dir(MENU_LED_PIN, GPIO_OUT);
    gpio_put(MENU_LED_PIN, 0);
    #endif

    const EventBits_t xKeyboardAllBitsToWaitFor =
        xKeyboardBitsToWaitFor |
        xKeyBoard_Key_A_BitsToWaitFor |
        xKeyBoard_Key_B_BitsToWaitFor |
        xKeyBoard_Key_C_BitsToWaitFor |
        xKeyBoard_Key_D_BitsToWaitFor |
        xKeyBoard_Key_sharp_BitsToWaitFor |
        xKeyBoard_Key_asterisco_BitsToWaitFor;

    for (;;) {
        uxBits = xEventGroupWaitBits(
            xMenuEventGroup,
            xKeyboardAllBitsToWaitFor,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);

        LOG_INFO("[Menu OLED Task] Evento recebido: 0x%08lx", (unsigned long)uxBits);

        switch (state) {
        case MENU_STATE_HIDDEN:
            if (uxBits & xKeyBoard_Key_sharp_BitsToWaitFor) {
                LOG_INFO("[Menu OLED Task] Abrindo menu.");
                #ifdef MENU_LED_PIN
                gpio_put(MENU_LED_PIN, 1);
                #endif
                menuOledOpen = true;
                state = MENU_STATE_NAVIGATE;
                menu_oled_render(selected_index);
            }
            break;

        case MENU_STATE_NAVIGATE:
            if (uxBits & xKeyBoard_Key_A_BitsToWaitFor) {
                selected_index = 0;
                menu_oled_render(selected_index);
            } else if (uxBits & xKeyBoard_Key_D_BitsToWaitFor) {
                selected_index = 1;
                menu_oled_render(selected_index);
            } else if (uxBits & xKeyBoard_Key_B_BitsToWaitFor) {
                if (selected_index > 0) {
                    selected_index--;
                    menu_oled_render(selected_index);
                }
            } else if (uxBits & xKeyBoard_Key_C_BitsToWaitFor) {
                if (selected_index < 1) {
                    selected_index++;
                    menu_oled_render(selected_index);
                }
            } else if (uxBits & xKeyBoard_Key_sharp_BitsToWaitFor) {
                if (selected_index == 0) {
                    LOG_INFO("[Menu OLED Task] Selecionado: Trocar senha");
                    password_edit_len = 0;
                    password_edit_buffer[0] = '\0';
                    state = MENU_STATE_EDIT_PASSWORD;
                    menu_oled_render_edit_value("Trocar senha", password_edit_buffer);
                } else if (selected_index == 1) {
                    LOG_INFO("[Menu OLED Task] Selecionado: Trocar nome");
                    name_edit_len = 0;
                    name_edit_buffer[0] = '\0';
                    state = MENU_STATE_EDIT_NAME;
                    menu_oled_render_edit_value("Trocar nome", name_edit_buffer);
                }
            }
            break;

        case MENU_STATE_EDIT_PASSWORD:
            handle_password_menu_keys(uxBits, &state, &selected_index);
            break;

        case MENU_STATE_EDIT_NAME:
            handle_name_menu_keys(uxBits, &state, &selected_index);
            break;
        }
    }
}

static void handle_password_menu_keys(EventBits_t uxBits, menu_state_t *state, int *selected_index) {
    (void) selected_index;

    if (uxBits & xKeyBoard_Key_asterisco_BitsToWaitFor) {
        LOG_INFO("[Menu OLED Task] Cancelando troca de senha, voltando ao menu.");
        *state = MENU_STATE_NAVIGATE;
        #ifdef MENU_LED_PIN
        gpio_put(MENU_LED_PIN, 0);
        #endif
        menuOledOpen = false;
        menu_oled_render(*selected_index);
        return;
    }

    if (uxBits & xKeyBoard_Key_D_BitsToWaitFor) {
        password_edit_len = 0;
        password_edit_buffer[0] = '\0';
        menu_oled_render_edit_value("Trocar senha", password_edit_buffer);
        return;
    }

    if (uxBits & xKeyBoard_Key_C_BitsToWaitFor) {
        if (password_edit_len > 0) {
            password_edit_len--;
            password_edit_buffer[password_edit_len] = '\0';
        }
        menu_oled_render_edit_value("Trocar senha", password_edit_buffer);
        return;
    }

    if ((uxBits & xKeyBoard_Key_sharp_BitsToWaitFor) ||
        (uxBits & xKeyBoard_Key_A_BitsToWaitFor)) {
        LOG_INFO("[Menu OLED Task] Senha confirmada (len=%d).", password_edit_len);
        *state = MENU_STATE_NAVIGATE;
        menu_oled_render(*selected_index);
        return;
    }

    if (uxBits & xKeyboard_Key_1_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '1';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_2_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '2';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_3_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '3';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_4_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '4';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_5_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '5';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_6_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '6';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_7_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '7';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_8_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '8';
            password_edit_buffer[password_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_9_BitsToWaitFor) {
        if (password_edit_len < (int)(sizeof(password_edit_buffer) - 1)) {
            password_edit_buffer[password_edit_len++] = '9';
            password_edit_buffer[password_edit_len] = '\0';
        }
    }

    menu_oled_render_edit_value("Trocar senha", password_edit_buffer);
}

static void handle_name_menu_keys(EventBits_t uxBits, menu_state_t *state, int *selected_index) {
    if (uxBits & xKeyBoard_Key_asterisco_BitsToWaitFor) {
        LOG_INFO("[Menu OLED Task] Cancelando troca de nome, voltando ao menu.");
        *state = MENU_STATE_NAVIGATE;

        #ifdef MENU_LED_PIN
        gpio_put(MENU_LED_PIN, 0);
        #endif
        menuOledOpen = false;

        menu_oled_render(*selected_index);
        return;
    }

    if (uxBits & xKeyBoard_Key_D_BitsToWaitFor) {
        name_edit_len = 0;
        name_edit_buffer[0] = '\0';
        menu_oled_render_edit_value("Trocar nome", name_edit_buffer);
        return;
    }

    if (uxBits & xKeyBoard_Key_C_BitsToWaitFor) {
        if (name_edit_len > 0) {
            name_edit_len--;
            name_edit_buffer[name_edit_len] = '\0';
        }
        menu_oled_render_edit_value("Trocar nome", name_edit_buffer);
        return;
    }

    if ((uxBits & xKeyBoard_Key_sharp_BitsToWaitFor) ||
        (uxBits & xKeyBoard_Key_A_BitsToWaitFor)) {
        LOG_INFO("[Menu OLED Task] Nome confirmado: %s", name_edit_buffer);
        if (name_edit_len > 0) {
            snprintf(rack_name, sizeof(rack_name), "%s", name_edit_buffer);
        }
        *state = MENU_STATE_NAVIGATE;

        #ifdef MENU_LED_PIN
        gpio_put(MENU_LED_PIN, 0);
        #endif
        menuOledOpen = false;

        menu_oled_render(*selected_index);
        return;
    }

    if (uxBits & xKeyboard_Key_1_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '1';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_2_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '2';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_3_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '3';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_4_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '4';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_5_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '5';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_6_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '6';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_7_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '7';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_8_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '8';
            name_edit_buffer[name_edit_len] = '\0';
        }
    } else if (uxBits & xKeyboard_Key_9_BitsToWaitFor) {
        if (name_edit_len < (int)(sizeof(name_edit_buffer) - 1)) {
            name_edit_buffer[name_edit_len++] = '9';
            name_edit_buffer[name_edit_len] = '\0';
        }
    }

    menu_oled_render_edit_value("Trocar nome", name_edit_buffer);
}

static void menu_oled_render(int selected_index) {
    const char *items[] = {"Trocar senha", "Trocar nome"};

    // Protege acesso ao OLED com semáforo para evitar contenção com outras tasks
    if (takeOled() != pdPASS) {
        LOG_WARN("[Menu OLED] Falha ao obter semáforo do OLED");
        return;
    }

    oled_clear();
    oled_set_text_line(0, "Menu", OLED_ALIGN_CENTER);

    for (int i = 0; i < 2; i++) {
        char line[32];
        if (i == selected_index) {
            snprintf(line, sizeof(line), "> %s", items[i]);
        } else {
            snprintf(line, sizeof(line), "  %s", items[i]);
        }
        oled_set_text_line(2 + i, line, OLED_ALIGN_LEFT);
    }

    oled_render_text();
    releaseOled();
}

static void menu_oled_render_edit(const char *title) {
    // Protege acesso ao OLED com semáforo para evitar contenção com outras tasks
    if (takeOled() != pdPASS) {
        LOG_WARN("[Menu OLED] Falha ao obter semáforo do OLED");
        return;
    }

    oled_clear();
    oled_set_text_line(0, title, OLED_ALIGN_CENTER);
    oled_set_text_line(2, "Use teclas para", OLED_ALIGN_CENTER);
    oled_set_text_line(3, "editar. * cancela", OLED_ALIGN_CENTER);
    oled_set_text_line(4, "# confirma/entra", OLED_ALIGN_CENTER);
    oled_render_text();
    releaseOled();
}

static void menu_oled_render_edit_value(const char *title, const char *value) {
    // Protege acesso ao OLED com semáforo para evitar contenção com outras tasks
    if (takeOled() != pdPASS) {
        LOG_WARN("[Menu OLED] Falha ao obter semáforo do OLED");
        return;
    }

    oled_clear();
    oled_set_text_line(0, title, OLED_ALIGN_CENTER);
    oled_set_text_line(2, value, OLED_ALIGN_LEFT);
    oled_set_text_line(3, "D=limpa C=back", OLED_ALIGN_CENTER);
    oled_set_text_line(4, "*=cancela #/A=ok", OLED_ALIGN_CENTER);
    oled_render_text();
    releaseOled();
}

bool menuOledIsOpen(void) {
    return menuOledOpen;
}
