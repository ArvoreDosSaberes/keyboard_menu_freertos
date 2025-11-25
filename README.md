# Biblioteca `keyboard_menu_freertos`

![Visitantes do Projeto](https://visitor-badge.laobi.icu/badge?page_id=arvoredossaberes.keyboard_menu_freertos)
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt6-Widgets-brightgreen)
![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-informational)
[![Docs](https://img.shields.io/badge/docs-Doxygen-blueviolet)](docs/index.html)
[![Latest Release](https://img.shields.io/github/v/release/ArvoreDosSaberes/keyboard_menu_freertos?label=version)](https://github.com/ArvoreDosSaberes/keyboard_menu_freertos/releases/latest)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-success.svg)](#contribuindo)

Biblioteca responsável por integrar o **teclado matricial** e o **menu exibido no display OLED** em um ambiente **FreeRTOS** para o firmware do *Rack Inteligente*.

Ela encapsula:
- **Leitura do teclado** e geração de eventos de tecla (`keyboard_task.c` / `keyboard_task.h`).
- **Gerenciamento do menu OLED** como uma *task* FreeRTOS (`menu_oled_task.c` / `menu_oled_task.h`).
- **Sincronização via Event Group** específico do menu (`menu_event_group.c` / `menu_event_group.h`).

A biblioteca é construída como uma *static library* CMake chamada `keyboard_menu_freertos`.

---

## Arquivos principais

- **`keyboard_task.c` / `keyboard_task.h`**  
  Implementam a `keyboard_task`, responsável por:
  - Ler o teclado (via biblioteca `keyboard`).
  - Traduzir as teclas em eventos.
  - Sinalizar o *event group* do menu quando houver interação.

- **`menu_oled_task.c` / `menu_oled_task.h`**  
  Implementam a `vMenuOledTask`, responsável por:
  - Desenhar as telas de menu no display OLED.
  - Reagir a eventos de teclado (via *event group* do menu).
  - Atualizar informações de status do rack conforme necessário.

- **`menu_event_group.c` / `menu_event_group.h`**  
  Fornecem:
  - Criação do *Event Group* do menu (`create_menu_event_group`).
  - Bits de eventos usados para sinalizar ações do teclado para o menu.

---

## Como a biblioteca é usada (exemplo baseado em `rack_inteligente.cpp`)

No arquivo `rack_inteligente.cpp`, após a inicialização básica de hardware, Wi‑Fi, OLED e grupos de eventos gerais, o código integra a biblioteca de teclado+menu da seguinte forma (resumo conceitual):

1. **Cria o Event Group do menu**:
   - Chamada `create_menu_event_group()` (definida em `menu_event_group.c`).
   - Em caso de erro, o firmware registra a falha no log e aborta a inicialização.

2. **Cria a task de teclado**:
   - Task `keyboard_task` (definida em `keyboard_task.c`).
   - É criada com `xTaskCreate`, usando *stack size* e prioridade definidas em `keyboard_menu_parameters.h`:
     - `KBD_POLL_TASK_STACK_SIZE`
     - `KBD_TASK_PRIORITY`

3. **Cria a task de menu OLED**:
   - Task `vMenuOledTask` (definida em `menu_oled_task.c`).
   - Também criada via `xTaskCreate` com parâmetros definidos em `keyboard_menu_parameters.h`:
     - `MENU_OLED_TASK_STACK_SIZE`
     - `MENU_OLED_TASK_PRIORITY`

4. **Integração com o restante do sistema**:
   - A task de teclado gera eventos (event bits) consumidos pela task de menu.
   - A task de menu exibe telas e opções no OLED, permitindo interação do usuário.

Depois disso, o firmware chama `vTaskStartScheduler()` e o FreeRTOS passa a gerenciar todas as tasks, incluindo teclado e menu.

---

## Como incluir a biblioteca no código-fonte

Para usar a biblioteca em um ponto semelhante ao `rack_inteligente.cpp`, você deve:

1. **Incluir os headers necessários** no seu arquivo C/C++ principal:

   - `menu_event_group.h` – para criar o *event group* do menu.
   - `keyboard_task.h` – para declarar a task do teclado.
   - `menu_oled_task.h` – para declarar a task do menu.
   - `keyboard_menu_parameters.h` – para *stack sizes*, prioridades e *delays* das tasks (já está incluído em `rack_inteligente.cpp`).

2. **Criar o Event Group do menu** na inicialização do sistema (antes de criar as tasks):
   - Chamar `create_menu_event_group()` e verificar o retorno.

3. **Criar as tasks** com `xTaskCreate`:
   - `keyboard_task` com os parâmetros corretos de *stack size* e prioridade.
   - `vMenuOledTask` com os parâmetros corretos de *stack size* e prioridade.

4. **Certificar-se de que o OLED já está inicializado** e que o semáforo de OLED está criado (via `initOledSemaphore()`), pois o menu faz uso do display.

---

## Arquivo de parâmetros `keyboard_menu_parameters.h`

Os parâmetros das tasks de teclado e de menu OLED são centralizados no header:

- Caminho sugerido: `inc/keyboard_menu_parameters.h`.
- Esse arquivo define, entre outros:
  - *Stack size* e prioridade da task de teclado (`keyboard_task`).
  - *Stack size* e prioridade da task de menu OLED (`vMenuOledTask`).
  - Delays de polling / atualização.
  - Pino de LED ou outros recursos auxiliares do menu.

Exemplo simplificado de conteúdo (baseado na implementação atual do firmware):

```c
#ifndef KEYBOARD_MENU_PARAMETERS_H
#define KEYBOARD_MENU_PARAMETERS_H

#include "FreeRTOS.h"
#include "event_groups.h"

#define KBD_POLL_TASK_STACK_SIZE       (configMINIMAL_STACK_SIZE)
#define KBD_TASK_PRIORITY              (tskIDLE_PRIORITY)
#define KBD_TASK_DELAY                 (500)
#define KBD_TIME_WAIT_DELAY            (1300000)

#define MENU_OLED_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define MENU_OLED_TASK_PRIORITY        (tskIDLE_PRIORITY + 5)
#define MENU_OLED_TASK_DELAY           (500)

#define MENU_LED_PIN 11

#endif // KEYBOARD_MENU_PARAMETERS_H
```

### Inclusão no projeto

1. **Criar o arquivo** `inc/keyboard_menu_parameters.h` no firmware (ou ajustar o existente) com os parâmetros apropriados para o seu sistema.
2. **Garantir que o diretório `inc` está no include path** do projeto (no exemplo, isso é feito em `target_include_directories(... ${CMAKE_SOURCE_DIR}/inc)`).
3. **Incluir o header** em qualquer módulo que precise desses parâmetros, tipicamente:

```c
#include "keyboard_menu_parameters.h"
```

Assim, é possível ajustar prioridades, tamanhos de pilha, tempos de *delay* e pinos relacionados ao menu sem modificar o código das bibliotecas.

---

## Integração com o `CMakeLists.txt`

A biblioteca é definida em `lib/keyboard_menu_freertos/CMakeLists.txt` da seguinte forma:

```cmake
add_library(keyboard_menu_freertos STATIC
    keyboard_task.c
    menu_oled_task.c
    menu_event_group.c
)

# Inclui headers deste módulo e do projeto
target_include_directories(keyboard_menu_freertos PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(keyboard_menu_freertos PUBLIC
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    keyboard
    rack_includes
    log
    oled
)

# Integrar com FreeRTOS (usa tasks/event groups)
link_freertos(keyboard_menu_freertos)
```

### 1. Adicionando a biblioteca ao projeto principal

No `CMakeLists.txt` principal do firmware (ou no CMakeLists da aplicação que contém o `main`), é necessário:

- **Incluir o subdiretório** da biblioteca (caso ainda não esteja):

```cmake
add_subdirectory(lib/keyboard_menu_freertos)
```

- **Linkar a biblioteca `keyboard_menu_freertos`** ao executável principal, por exemplo:

```cmake
target_link_libraries(rack_inteligente
    PRIVATE
        keyboard_menu_freertos
)
```

> Substitua `rack_inteligente` pelo nome do seu *target* executável, se for diferente.

### 2. Dependências já resolvidas pela própria biblioteca

O `CMakeLists.txt` da biblioteca já faz o link com:

- `FreeRTOS-Kernel-Heap4`
- `pico_stdlib`
- `keyboard`
- `rack_includes`
- `log`
- `oled`

Ou seja, ao linkar `keyboard_menu_freertos` no seu executável, essas dependências também serão trazidas automaticamente (pois estão em `target_link_libraries(keyboard_menu_freertos PUBLIC ...)`).

### 3. Incluindo headers

A diretiva:

```cmake
target_include_directories(keyboard_menu_freertos PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/inc
)
```

garante que, ao linkar `keyboard_menu_freertos`, você possa fazer `#include` dos arquivos de cabeçalho da biblioteca diretamente, como por exemplo:

```c
#include "menu_event_group.h"
#include "keyboard_task.h"
#include "menu_oled_task.h"
```

sem precisar ajustar manualmente o `include_path` no CMake do executável.

---

## Fluxo típico de inicialização usando esta biblioteca

1. Inicializar **log**, **Wi‑Fi**, **I2C**, **OLED** e **semafaro do OLED** (`initOledSemaphore`).
2. Criar *event groups* gerais do rack (ex.: `create_event_group`).
3. Criar o *event group* do menu: `create_menu_event_group`.
4. Criar as tasks:
   - `keyboard_task` (teclado).
   - `vMenuOledTask` (menu OLED).
5. Criar demais tasks (sensores, MQTT, sinais, etc.).
6. Chamar `vTaskStartScheduler()`.

Seguindo esse fluxo, o teclado e o menu OLED passam a funcionar como parte integrada do sistema FreeRTOS do Rack Inteligente.

---

## Checklist de configuração rápida

Para integrar a biblioteca `keyboard_menu_freertos` em outro projeto, verifique:

1. **Bibliotecas base disponíveis**
   - [ ] A biblioteca `keyboard` está adicionada e funcionando (ver README de `lib/keyboard`).
   - [ ] As bibliotecas de OLED, log e FreeRTOS já estão integradas (`oled`, `log`, `FreeRTOS-Kernel-Heap4`).

2. **Headers de parâmetros criados**
   - [ ] Existe o arquivo `inc/keyboard_menu_parameters.h` com defines de stack, prioridade e delays das tasks (`KBD_*`, `MENU_OLED_*`, `MENU_LED_PIN`).

3. **Include path**
   - [ ] O diretório `inc` está no include path do seu executável (por exemplo via `${CMAKE_SOURCE_DIR}/inc`).

4. **Subdiretório da biblioteca adicionado**
   - [ ] O `CMakeLists.txt` principal contém:

```cmake
add_subdirectory(lib/keyboard_menu_freertos)
```

5. **Link da biblioteca**
   - [ ] O seu *target* principal linka com `keyboard_menu_freertos`:

```cmake
target_link_libraries(seu_executavel
    PRIVATE
        keyboard_menu_freertos
)
```

6. **Includes no código**
   - [ ] O arquivo principal (por exemplo, o que contém `main`) inclui:

```c
#include "menu_event_group.h"
#include "keyboard_task.h"
#include "menu_oled_task.h"
#include "keyboard_menu_parameters.h"
```

7. **Inicialização em tempo de execução**
   - [ ] O *event group* geral (`create_event_group`) e o do menu (`create_menu_event_group`) são criados antes das tasks.
   - [ ] O OLED foi inicializado e o semáforo criado (`initOledSemaphore`).
   - [ ] As tasks `keyboard_task` e `vMenuOledTask` são criadas com os parâmetros definidos em `keyboard_menu_parameters.h`.

Se todos os itens acima estiverem marcados, a biblioteca `keyboard_menu_freertos` deve compilar e funcionar corretamente como parte do seu sistema FreeRTOS.
