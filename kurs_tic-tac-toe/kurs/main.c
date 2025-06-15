#define _CRT_SECURE_NO_WARNINGS
#include "include/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define STB_TRUETYPE_IMPLEMENTATION
#include "text/stb_truetype.h"
// Библиотека для отрисовки буковок и чиселок, дополнительно подгружен шрифт arial.ttf



                                                    /*                                СТРУКТУРЫ                                     */
// Состояния меню приложения
typedef enum {
    MENU_MAIN,     // Главное меню
    MENU_GAME,     // Игровой экран
    MENU_ABOUT,    // Экран "О программе"
    MENU_SETTINGS  // Экран настроек
} AppMenuState;

// Уровни сложности игры
typedef enum {
    DIFFICULTY_EASY,    // Легкий
    DIFFICULTY_MEDIUM,  // Средний
    DIFFICULTY_HARD,    // Сложный
    DIFFICULTY_EXPERT   // Эксперт
} GameDifficulty;

typedef enum {
    GAME_RESULT_NONE = 0,   // Игра продолжается
    GAME_RESULT_WIN = 1,    // Победа
    GAME_RESULT_LOSE = -1,  // Поражение
    GAME_RESULT_DRAW = 2    // Ничья
} GameResultType;

// Тип хода (игрок/бот)
typedef enum {
    MOVE_PLAYER,  // Ход игрока (крестик)
    MOVE_AI       // Ход бота (нолик)
} MoveType;

typedef enum {
    FIRST_MOVE_PLAYER,  // Игрок ходит первым (крестики)
    FIRST_MOVE_AI       // Бот ходит первым (нолики)
} FirstMove;

// Элемент очереди
typedef struct MoveLog {
    int x, y;           // Координаты
    MoveType type;       // Тип хода
    struct MoveLog* next;
} MoveLog;

// Очередь (FIFO)
typedef struct {
    MoveLog* head;       // Первый элемент
    MoveLog* tail;       // Последний элемент
    int count;           // Текущее количество (макс 6)
} MoveLogger;

// Камера для управления видом игрового поля
typedef struct {
    float zoom;     // Масштаб
    float offsetX;  // Смещение по X
    float offsetY;  // Смещение по Y
} Camera;

// Состояние мыши
typedef struct {
    double lastX;       // Последняя позиция X
    double lastY;       // Последняя позиция Y
    int isDragging:2;     // Флаг перетаскивания
} MouseState;

// Клетка игрового поля
typedef struct {
    int x, y;          // Координаты
    short int symbol:3;   // Символ (0 - пусто, 1 - крестик, 2 - нолик)
} Cell;

// Игровое поле
typedef struct {
    Cell* cells;    // Массив клеток
    int size;       // Текущий размер
    int capacity;   // Выделенная память
} Grid;

// Настройки игры
typedef struct {
    GameDifficulty difficulty;  // Уровень сложности(4)
    int fieldSize;             // Размер поля (0 - бесконечное)
    int winLineLength;         // Длина выигрышной линии (минимум 3)
    FirstMove firstMove;       // Кто ходит первым
} GameSettings;

// Основное состояние приложения
typedef struct {
    Camera camera;              // Камера
    MouseState mouse;           // Состояние мыши
    Grid grid;                  // Игровое поле
    int selectedCellX;          // Выбранная клетка X
    int selectedCellY;          // Выбранная клетка Y
    AppMenuState currentState;  // Текущее состояние меню
    int menuSelectedItem:3;       // Выбранный пункт главного меню
    int showHelp:2;               // Показать справку
    int settingsSelectedItem:3;   // Выбранный пункт настроек

    stbtt_bakedchar cdata[96];  // Данные шрифта
    GLuint fontTexture;         // Текстура шрифта
    float fontSize;             // Размер шрифта
    float saveNotificationTimer;// Таймер уведомления о сохранении

    GameSettings settings;       // Текущие настройки
    GameSettings defaultSettings; // Настройки из settings.txt


    union {
        int rawResult;                  // Доступ как к числу (для совместимости)
        GameResultType resultType;       // Типизированный доступ
        struct {
            unsigned int isWin : 1;        // Флаг победы
            unsigned int isLose : 1;       // Флаг поражения
            unsigned int isDraw : 1;       // Флаг ничьи
        };
    } gameResult;
    int winLineStartX;       // Начало линии (координаты клетки)
    int winLineStartY;
    int winLineEndX;         // Конец линии (координаты клетки)
    int winLineEndY;

    MoveLogger logger;  // Логгер ходов
    int showMoveLog;    // Флаг для отображения окна (1 - показать, 0 - скрыть)
} AppState;


// Прототипы функций для ботов
void makeAIMoveEasy(AppState* state);
void makeAIMoveMedium(AppState* state);
void makeAIMoveHard(AppState* state);
void makeAIMoveExpert(AppState* state);





                                            /*                                     ГРАФИКА                                                */




// Прототип для сохранений
void saveSettings(const GameSettings* settings);

// Прототип чеков
int checkWinCondition(AppState* state, int symbol, int winLength);
int checkForDraw(AppState* state);

// Инициализация состояния приложения
void initAppState(AppState* state) {
    // Настройки камеры
    state->camera.zoom = 1.0f;
    state->camera.offsetX = 0.0f;
    state->camera.offsetY = 0.0f;

    // Состояние мыши
    state->mouse.lastX = 0.0;
    state->mouse.lastY = 0.0;
    state->mouse.isDragging = 0;

    // Игровое поле
    state->grid.cells = NULL;
    state->grid.size = 0;
    state->grid.capacity = 0;

    // Выбранная клетка
    state->selectedCellX = 0;
    state->selectedCellY = 0;

    // Состояние интерфейса
    state->currentState = MENU_MAIN;
    state->showHelp = 0;
    state->menuSelectedItem = 0;
    state->settingsSelectedItem = 0;
    state->fontTexture = 0;
    state->fontSize = 32.0f;
    state->saveNotificationTimer = 0.0f;

    // Логгер ходов
    state->logger.head = NULL;
    state->logger.tail = NULL;
    state->logger.count = 0;
    state->showMoveLog = 0;  // По умолчанию окно скрыто

    
    
    // Настройки по умолчанию
    state->settings.difficulty = DIFFICULTY_EASY;
    state->settings.fieldSize = 0;      // Бесконечное поле
    state->settings.winLineLength = 3;  // Выигрышная линия из 3 символов
    state->settings.firstMove = FIRST_MOVE_PLAYER; // По умолчанию игрок ходит первым
    FILE* settingsFile = fopen("settings.txt", "r");
    if (settingsFile) {
        char line[256];
        while (fgets(line, sizeof(line), settingsFile)) {
            if (strncmp(line, "difficulty=", 11) == 0) {
                int diff = atoi(line + 11);
                if (diff >= DIFFICULTY_EASY && diff <= DIFFICULTY_EXPERT) {
                    state->settings.difficulty = diff;
                }
            }
            else if (strncmp(line, "fieldSize=", 10) == 0) {
                state->settings.fieldSize = atoi(line + 10);
            }
            else if (strncmp(line, "winLineLength=", 14) == 0) {
                int length = atoi(line + 14);
                state->settings.winLineLength = (length >= 3) ? length : 3;
            }
            else if (strncmp(line, "firstMove=", 10) == 0) {
                int move = atoi(line + 10);
                if (move == FIRST_MOVE_PLAYER || move == FIRST_MOVE_AI) {
                    state->settings.firstMove = move;
                }
            }
        }
        fclose(settingsFile);
    }
    else {
        // Если файла нет, создать его с дефолтными настройками
        saveSettings(&state->settings);
    }

    // Установка текущих настройки равными дефолтным
    state->defaultSettings = state->settings;

    state->gameResult.rawResult = 0;          // 0 - нет результата, 1 - победа, -1 - поражение
    state->winLineStartX = 0;       // Начало линии (координаты клетки)
    state->winLineStartY = 0;
    state->winLineEndX = 0;         // Конец линии (координаты клетки)
    state->winLineEndY = 0;
}



// Сохранение настроек в файл
void saveSettings(const GameSettings* settings) {
    FILE* file = fopen("settings.txt", "w");
    if (!file) {
        fprintf(stderr, "Failed to save settings\n");
        return;
    }

    fprintf(file, "difficulty=%d\n", settings->difficulty);
    fprintf(file, "fieldSize=%d\n", settings->fieldSize);
    fprintf(file, "winLineLength=%d\n", settings->winLineLength);
    fprintf(file, "firstMove=%d\n", settings->firstMove);

    fclose(file);
}

// Очищение поля
void cleanupGrid(Grid* grid) {
    if (grid->cells) {
        free(grid->cells);
        grid->cells = NULL;
    }
    grid->size = 0;
    grid->capacity = 0;
  
}



void cleanupMoveLogger(MoveLogger* logger) {
    if (!logger) return;

    MoveLog* current = logger->head;
    while (current != NULL) {
        MoveLog* next = current->next;  // Копируем next до free
        free(current);
        current = next;
    }

    logger->head = NULL;
    logger->tail = NULL;
    logger->count = 0;
}



void logMove(MoveLogger* logger, int x, int y, MoveType type) {
    MoveLog* newMove = (MoveLog*)malloc(sizeof(MoveLog));
    if (newMove != NULL) {
        newMove->x = x;
        newMove->y = y;
        newMove->type = type;
        newMove->next = NULL;
    }

    if (logger->count >= 6) {
        // Удаляем самый старый ход (FIFO)
        MoveLog* temp = logger->head;
        logger->head = logger->head->next;
        free(temp);
        logger->count--;
    }

    if (logger->head == NULL) {
        logger->head = newMove;
        logger->tail = newMove;
    }
    else {
        logger->tail->next = newMove;
        logger->tail = newMove;
    }
    logger->count++;
}


// Функция добавления клетки
void addCell(Grid* grid, int x, int y) {
    //  Проверка входного параметра
    if (grid == NULL) return;

    //  Проверка на дубликаты
    for (int i = 0; i < grid->size; ++i) {
        if (grid->cells[i].x == x && grid->cells[i].y == y) {
            return;
        }
    }

    //  Расширение массива при необходимости
    if (grid->size >= grid->capacity) {
        const int newCapacity = (grid->capacity == 0) ? 4 : grid->capacity * 2;

        // Создаем временный указатель для анализатора
        Cell* const newCells = (Cell*)realloc(grid->cells, newCapacity * sizeof(Cell));
        if (!newCells) {
            fprintf(stderr, "Memory allocation failed\n");
            cleanupGrid(grid);
            exit(1);
        }

        grid->cells = newCells;
        grid->capacity = newCapacity;
    }

    //  Костыль для VS2019 - делаем запись через указатель
    Cell* target = grid->cells + grid->size;
    target->x = x;
    target->y = y;
    target->symbol = 0;

    //  Увеличиваем размер только после успешной записи
    grid->size++;
}


// Инициализация и создание текста
int initText(AppState* state, const char* fontPath) {
    FILE* fontFile = fopen(fontPath, "rb");
    if (!fontFile) {
        fprintf(stderr, "Incorrect file for text\n");
        return 0;
    }

    fseek(fontFile, 0, SEEK_END);
    long size = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);

    unsigned char* fontBuffer = (unsigned char*)malloc(size);
    if (!fontBuffer) {
        fclose(fontFile);
        return 0;
    }

    fread(fontBuffer, 1, size, fontFile);
    fclose(fontFile);

    int texWidth = 512;
    int texHeight = 512;
    unsigned char* tempBitmap = (unsigned char*)malloc((long long int)texWidth * texHeight);
    if (!tempBitmap) {
        free(fontBuffer);
        return 0;
    }

    stbtt_BakeFontBitmap(fontBuffer, 0, state->fontSize,
        tempBitmap, texWidth, texHeight, 32, 96, state->cdata);

    // Создание текстуры
    glGenTextures(1, &state->fontTexture);
    glBindTexture(GL_TEXTURE_2D, state->fontTexture);

    
    unsigned char* rgbaBitmap = (unsigned char*)malloc((long long int)texWidth * texHeight * 4);
    for(int i = 0; i < texWidth * texHeight; i++) {
        if (rgbaBitmap == NULL) {
            break;
        }
        rgbaBitmap[i*4] = 255;
        rgbaBitmap[i*4+1] = 255;
        rgbaBitmap[i*4+2] = 255;
        rgbaBitmap[i*4+3] = tempBitmap[i];
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
                0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBitmap);
    free(rgbaBitmap);
    

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(tempBitmap);
    free(fontBuffer);
    return 1;
}


// Функция сохранения игры
void saveGame(const Grid* grid, const GameSettings* settings) {
    FILE* file = fopen("saves.txt", "w");
    if (!file) {
        fprintf(stderr, "Incorrect file for saving\n");
        return;
    }

    // Сохраняем настройки первой строкой, добавляем firstMove
    fprintf(file, "SETTINGS %d %d %d %d\n",
        settings->difficulty,
        settings->fieldSize,
        settings->winLineLength,
        settings->firstMove);  // Добавляем сохранение порядка хода

    // Затем сохраняем клетки
    for (int i = 0; i < grid->size; i++) {
        fprintf(file, "%d %d %d\n", grid->cells[i].x, grid->cells[i].y, grid->cells[i].symbol);
    }

    fclose(file);
}


// Загрузка игры
int loadGame(Grid* grid, GameSettings* settings) {
    FILE* file = fopen("saves.txt", "r");
    if (!file) {
        file = fopen("saves.txt", "w");
        if (file) fclose(file);
        return 0;
    }

    cleanupGrid(grid);

    // Настройки из первой строки файла
    char line[256];
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SETTINGS", 8) == 0) {
            int temp1;
            int temp2;
            int check = sscanf(line + 8, "%d %d %d %d",
                &temp1,
                &settings->fieldSize,
                &settings->winLineLength,
                &temp2);  // Загружаем порядок хода
            settings->difficulty = (GameDifficulty)temp1;
            settings->firstMove = (FirstMove)temp2;

            // Если не удалось прочитать firstMove (старый формат файла)
            if (check < 4) {
                settings->firstMove = FIRST_MOVE_PLAYER; // Устанавливаем по умолчанию
            }
        }
        else {
            // Если файл старого формата, переходим в начало
            fseek(file, 0, SEEK_SET);
            settings->firstMove = FIRST_MOVE_PLAYER; // Устанавливаем по умолчанию
        }
    }

    int x, y, symbol;
    while (fscanf(file, "%d %d %d", &x, &y, &symbol) == 3) {
        addCell(grid, x, y);
        grid->cells[grid->size - 1].symbol = symbol;
    }

    fclose(file);
    return 1;
}


// Рендер текста
void renderText(AppState* state, const char* text, float x, float y, float scale, float r, float g, float b, float a) {
    if (!state->fontTexture) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, state->fontTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(x, y, 0);
    glScalef(scale, -scale, 1.0f); 

    glBegin(GL_QUADS);
    glColor4f(r, g, b, a); 

    float startX = 0;
    float drawY = 0;
    while (*text) {
        
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(state->cdata, 512, 512, *text - 32, &startX, &drawY, &q, 1);

            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
        
        ++text;
    }

    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// Отрисовка уведомления сохранения
void drawSaveNotification(AppState* state, int width, int height) {
    if (state->saveNotificationTimer <= 0.0f) return;

    // Сохраняем текущую матрицу проекции
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Размеры и позиция уведомления
    float notifWidth = 300;
    float notifHeight = 60;
    float notifX = (width - notifWidth) / 2;
    float notifY = height - 100.0f;

    // Рисуем полупрозрачный фон
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.5f, 0.1f, 0.9f * (state->saveNotificationTimer / 1.0f)); // Плавное исчезновение
    glBegin(GL_QUADS);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();
    glDisable(GL_BLEND);

    // Рамка
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.8f, 1.0f, 0.8f, 0.9f * (state->saveNotificationTimer / 1.0f));
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();
    glDisable(GL_BLEND);

    // Текст уведомления
    float textAlpha = state->saveNotificationTimer / 1.0f;
    renderText(state, "Game saved successfully!",
        notifX + 20, notifY + 35,
        0.7f, 1.0f, 1.0f, 1.0f, textAlpha); 

    // Восстанавливаем матрицы
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


// Отрисовка главного меню
void drawMainMenu(AppState* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Фон меню
    glColor3f(0.1f, 0.1f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // Заголовок
    float titleWidth = 0;
    const char* title = "Krestiki-Noliki";
    const char* p = title;
    while (*p) {
        
            titleWidth += state->cdata[*p - 32].xadvance;
        
        p++;
    }
    renderText(state, title, (1240 - titleWidth * 1.5f) / 2, 1000, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Кнопки меню
    const char* items[] = { "New Game", "Load Game", "Settings", "About" };
    for (int i = 0; i < 4; i++) {
        float x = 500;
        float y = 800.0f - i * 200.0f;
        float width = 240;
        float height = 80;

        // Рамка кнопки
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();

        // Заливка кнопки
        if (i == state->menuSelectedItem) {
            glColor3f(0.3f, 0.3f, 0.6f);
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f);
        }
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 2);
        glVertex2f(x + width - 2, y + 2);
        glVertex2f(x + width - 2, y + height - 2);
        glVertex2f(x + 2, y + height - 2);
        glEnd();

        // Текст кнопки
        float textWidth = 0;
        p = items[i];
        while (*p) {
            
                textWidth += state->cdata[*p - 32].xadvance;
            
            p++;
        }
        float textX = x + (width - textWidth * 0.8f) / 2;
        float textY = y + (height - state->fontSize * 0.6f) / 2 + state->fontSize * 0.5f;
        renderText(state, items[i], textX, textY, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}

// Отрисовка меню об авторах
void drawAboutScreen(AppState* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Фон
    glColor3f(0.1f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // Заголовок
    float titleWidth = 0;
    const char* title = "About";
    const char* p = title;
    while (*p) {
        
            titleWidth += state->cdata[*p - 32].xadvance;
        
        p++;
    }
    renderText(state, title, (1240 - titleWidth * 1.5f) / 2, 1000, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Текст
    const char* lines[] = {
        "Krestiki-Noliki Game",
        "Version 1.0 2025y.",
        "Autors: Galko Nikolay Romanovich, Panova Victoria Maximovna",
        "Peter the Great St.Petersburg Polytechnic University",
        "Institute of Computer Technology and Cybersecurity",
        "Graduate School of Cybersecurity",
        "Group 5131001/40002",
        "ESC - Back"
    };

    for (int i = 0; i < 8; i++) {
        float textWidth = 0;
        p = lines[i];
        while (*p) {
            
                textWidth += state->cdata[*p - 32].xadvance;
            
            p++;
        }
        float x = (1240 - textWidth * 1.0f) / 2;
        float y = 900.0f - i * 100.0f;
        renderText(state, lines[i], x, y, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}


//  функция отрисовки экрана настроек
void drawSettingsScreen(AppState* state, GLFWwindow* window) {
    // Получаем координаты мыши
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseY = height - mouseY; // Инвертируем Y
    // Масштабируем к виртуальным координатам 1240x1240
    mouseX = (mouseX / width) * 1240.0f;
    mouseY = (mouseY / height) * 1240.0f;

    // Установка матрицы проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Фон
    glColor3f(0.2f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // Заголовок
    renderText(state, "Settings", 500, 1100, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Координаты и размеры элементов
    float yPos = 800;
    float buttonWidth = 120;
    float buttonHeight = 50;
    float arrowSize = 20;
    float valueBoxX = 700;

    //  Настройка сложности
    renderText(state, "Difficulty Level:", 300.0f, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    const char* difficultyOptions[] = { "Easy", "Medium", "Hard", "Expert" };
    for (int i = 0; i < 4; i++) {
        float xPos = 600.0f + i * 130.0f;

        // Проверка наведения мыши
        int isHovered = (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

        // Цвет кнопки
        if ((int)state->settings.difficulty == i) {
            glColor3f(0.4f, 0.4f, 0.8f); // Выбранный вариант
        }
        else if (isHovered) {
            glColor3f(0.3f, 0.3f, 0.6f); // Наведение
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f); // Обычный
        }

        // Рисуем кнопку
        glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Рамка
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Текст
        float textWidth = 0;
        const char* p = difficultyOptions[i];
        while (*p) {
             textWidth += state->cdata[*p - 32].xadvance;
            p++;
        }
        renderText(state, difficultyOptions[i],
            xPos + (buttonWidth - textWidth * 0.7f) / 2,
            yPos + (buttonHeight - state->fontSize * 0.6f) / 2 + state->fontSize * 0.5f,
            0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    //  Размер поля
    yPos -= 120;
    renderText(state, "Field Size (0=infinite):", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    char fieldSizeText[32];
    snprintf(fieldSizeText, sizeof(fieldSizeText), "%d", state->settings.fieldSize);

    // Проверка наведения на кнопки
    int decHovered = (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);
    int incHovered = (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

    // Кнопка уменьшения
    glColor3f(decHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX - 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Поле значения
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Кнопка увеличения
    glColor3f(incHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX + buttonWidth + 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Рамка
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Текст значения
    float textWidth = 0;
    const char* p = fieldSizeText;
    while (*p) {
        textWidth += state->cdata[*p - 32].xadvance;
        p++;
    }
    renderText(state, fieldSizeText,
        valueBoxX + (buttonWidth - textWidth * 0.7f) / 2,
        yPos + (buttonHeight - state->fontSize * 0.6f) / 2 + state->fontSize * 0.5f,
        0.7f, 1.0f, 1.0f, 1.0f, 1.0f);

    //  Длина выигрышной линии
    yPos -= 120;
    renderText(state, "Win Line Length:", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    char winLineText[32];
    snprintf(winLineText, sizeof(winLineText), "%d", state->settings.winLineLength);

    // Проверка наведения на кнопки
    decHovered = (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);
    incHovered = (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

    // Кнопка уменьшения
    glColor3f(decHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX - 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Поле значения
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Кнопка увеличения
    glColor3f(incHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX + buttonWidth + 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Рамка
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Текст значения
    textWidth = 0;
    p = winLineText;
    while (*p) {
        textWidth += state->cdata[*p - 32].xadvance;
        p++;
    }
    renderText(state, winLineText,
        valueBoxX + (buttonWidth - textWidth * 0.7f) / 2,
        yPos + (buttonHeight - state->fontSize * 0.6f) / 2 + state->fontSize * 0.5f,
        0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
    yPos -= 120;
    renderText(state, "First Move:", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    const char* firstMoveOptions[] = { "Player", "AI" };
    for (int i = 0; i < 2; i++) {
        float xPos = 600.0f + i * 130.0f;

        // Проверка наведения мыши
        int isHovered = (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

        // Цвет кнопки
        if ((int)state->settings.firstMove == i) {
            glColor3f(0.4f, 0.4f, 0.8f); // Выбранный вариант
        }
        else if (isHovered) {
            glColor3f(0.3f, 0.3f, 0.6f); // Наведение
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f); // Обычный
        }

        // Рисуем кнопку
        glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Рамка
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Текст
        textWidth = 0;
        p = firstMoveOptions[i];
        while (*p) {
            textWidth += state->cdata[*p - 32].xadvance;
            p++;
        }
        renderText(state, firstMoveOptions[i],
            xPos + (buttonWidth - textWidth * 0.7f) / 2,
            yPos + (buttonHeight - state->fontSize * 0.6f) / 2 + state->fontSize * 0.5f,
            0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    //  Кнопка сохранения
    yPos -= 150;
    float saveX = (1240 - 300) / 2;
    int saveHovered = (mouseX >= saveX && mouseX <= (double)saveX + 300 &&
        mouseY >= yPos && mouseY <= (double)yPos + 60);

    // Фон кнопки
    glColor3f(saveHovered ? 0.4f : 0.3f, saveHovered ? 0.8f : 0.6f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(saveX, yPos);
    glVertex2f(saveX + 300, yPos);
    glVertex2f(saveX + 300, yPos + 60);
    glVertex2f(saveX, yPos + 60);
    glEnd();

    // Рамка
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(saveX, yPos);
    glVertex2f(saveX + 300, yPos);
    glVertex2f(saveX + 300, yPos + 60);
    glVertex2f(saveX, yPos + 60);
    glEnd();

    // Текст
    renderText(state, "Save Settings", saveX + 70, yPos + 30, 0.9f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Кнопка назад
    renderText(state, "Back (ESC)", 50, 50, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
}


// Полная функция обработки кликов в настройках
void handleSettingsClick(AppState* state, GLFWwindow* window, int button) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    // Получаем координаты мыши
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseY = height - mouseY; // Инвертируем Y
    // Масштабируем к виртуальным координатам 1240x1240
    mouseX = (mouseX / width) * 1240.0f;
    mouseY = (mouseY / height) * 1240.0f;

    // Координаты элементов
    float yPos = 800;
    float buttonWidth = 120;
    float buttonHeight = 50;
    float valueBoxX = 700;

    //  Проверка кликов по уровню сложности
    for (int i = 0; i < 4; i++) {
        float xPos = 600.0f + i * 130.0f;
        if (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
            state->settings.difficulty = i;
            return;
        }
    }

    //  Проверка кликов по размеру поля
    yPos -= 120;

    // Кнопка уменьшения
    if (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
        if (state->settings.fieldSize <= 1) {
            state->settings.fieldSize = 0;
        }
        else {
            state->settings.fieldSize -= 2;
        }
        return;
    }

    // Кнопка увеличения
    if (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
        if (state->settings.fieldSize == 0) {
            state->settings.fieldSize++;
        }
        else {
            state->settings.fieldSize += 2;
        }
        return;
    }
    if (state->settings.winLineLength > state->settings.fieldSize && state->settings.fieldSize != 0) {
        state->settings.winLineLength = state->settings.fieldSize;
    }

    //  Проверка кликов по длине линии
    yPos -= 120;

    // Кнопка уменьшения
    if (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
        state->settings.winLineLength = (state->settings.winLineLength > 3) ? state->settings.winLineLength - 1 : 3;
        return;
    }

    // Кнопка увеличения
    if ((float)mouseX >= valueBoxX + buttonWidth + 10 && (float)mouseX <= valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
        if (state->settings.winLineLength < state->settings.fieldSize || state->settings.fieldSize == 0) {
            state->settings.winLineLength++;
        }
        return;
    }

    yPos -= 120;
    for (int i = 0; i < 2; i++) {
        float xPos = 600.0f + i * 130.0f;
        if (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
            state->settings.firstMove = i;
            return;
        }
    }
    

    //  Проверка клика по кнопке сохранения
    yPos -= 150;
    float saveX = (1240 - 300) / 2;
    if (mouseX >= saveX && mouseX <= (double)saveX + 300 &&
        mouseY >= yPos && mouseY <= (double)yPos + 60) {
        saveSettings(&state->settings);
        state->defaultSettings = state->settings; // Обновляем дефолтные настройки
        return;
    }

    //  Проверка клика по кнопке назад (верхний левый угол)
    if (mouseX < 200 && mouseY < 100) {
        state->currentState = MENU_MAIN;
    }
}


// Замена виндовской функции _itoa т.к. для линуха низя
static void int_to_str(int value, char* str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char* ptr = str;
    int is_negative = 0;

    // Обработка отрицательных чисел для base 10
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    // Обрабатываем 0 явно, иначе будет пустая строка
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    // Записываем цифры в обратном порядке
    char* start = ptr;
    while (value != 0) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
        value /= base;
    }

    // Добавляем знак минус для отрицательных чисел
    if (is_negative) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    // Разворачиваем строку
    ptr--;
    while (start < ptr) {
        char tmp = *start;
        *start++ = *ptr;
        *ptr-- = tmp;
    }
}


// Отрисовка окна помощи
void drawHelpWindow(AppState* state) {
    // Сохраняем текущую матрицу проекции
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Рисуем полупрозрачный фон
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.1f, 0.2f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(300, 170);
    glVertex2f(940, 170);
    glVertex2f(940, 900);
    glVertex2f(300, 900);
    glEnd();
    glDisable(GL_BLEND);

    // Рамка
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(300, 170);
    glVertex2f(940, 170);
    glVertex2f(940, 900);
    glVertex2f(300, 900);
    glEnd();

    // Заголовок
    renderText(state, "Game Controls", 450, 850, 1.2f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Текст инструкций
    const char* instructions[] = {
        "WASD or Arrow Keys - Move cursor",
        "Shift - Center view on selected",
        "Space - Place X",
        "Alt+Q - Save game",
        "ESC - Return to main menu",
        "M - Show logs of moves",
        "",
        "Current Settings:",
        "Difficulty: ",
        "Field Size: ",
        "Win Line Length: ",
        "Press H to close"
    };

    // Получаем текстовые представления настроек
    const char* difficultyNames[] = { "Easy", "Medium", "Hard", "Expert" };
    char fieldSizeText[32];
    char winLineText[32];
    char modetext[32];
    int_to_str(state->settings.fieldSize, modetext, 10);
    snprintf(fieldSizeText, sizeof(fieldSizeText), "%s",
        state->settings.fieldSize == 0 ? "Infinite" : modetext);
    int_to_str(state->settings.winLineLength, winLineText, 10);

    for (int i = 0; i < 6; i++) {
        renderText(state, instructions[i], 320.0f, 750.0f - i * 50.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Отрисовываем настройки
    renderText(state, instructions[6], 320, 450, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Сложность
    float x = 320;
    float y = 400;
    renderText(state, instructions[7], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, difficultyNames[state->settings.difficulty], x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Размер поля
    y -= 50;
    renderText(state, instructions[8], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, fieldSizeText, x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Длина линии
    y -= 50;
    renderText(state, instructions[9], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, winLineText, x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Остальной текст
    renderText(state, instructions[11], 320, 200, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Восстанавливаем матрицы
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void drawMoveLog(AppState* state) {
    // 1. Настройка матриц и прозрачности
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1); // Размер окна 1240x1240
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 2. Параметры окна
    float x = 50.0f, y = 100.0f;
    float width = 300.0f, height = 200.0f;

    // 3. Фон окна (полупрозрачный)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.1f, 0.2f, 0.9f); // Темно-синий с прозрачностью
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 4. Рамка окна
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f); // Белая
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 5. Текст (заголовок)
    renderText(state, "Last Moves:", x + 10.0f, y + height - 30.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // 6. Список ходов
    MoveLog* current = state->logger.head;
    float textY = y + height - 60.0f; // Стартовая позиция для текста
    int count = 0;

    while (current != NULL && count < 6) {
        char buf[50];
        const char* type = (current->type == MOVE_PLAYER) ? "Player" : "AI";
        snprintf(buf, sizeof(buf), "%s: (%d, %d)", type, current->x, current->y);

        // Проверяем, не вышли ли за границы окна
        if (textY < y + 10.0f) break;

        renderText(state, buf, x + 10.0f, textY, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
        textY -= 25.0f; // Смещаем вниз
        current = current->next;
        count++;
    }

    // 7. Восстанавливаем матрицы
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_BLEND);
}


// Подсказка в правом верхнем углу о помощи
void drawHelpHint(AppState* state, int width, int height) {
    // Сохраняем текущую матрицу проекции
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Размеры и позиция блока подсказки
    float hintWidth = 200;
    float hintHeight = 50;
    float hintX = width - hintWidth - 20;
    float hintY = height - hintHeight - 20;

    // Рисуем полупрозрачный фон
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.1f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(hintX, hintY);
    glVertex2f(hintX + hintWidth, hintY);
    glVertex2f(hintX + hintWidth, hintY + hintHeight);
    glVertex2f(hintX, hintY + hintHeight);
    glEnd();
    glDisable(GL_BLEND);

    // Рамка
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(hintX, hintY);
    glVertex2f(hintX + hintWidth, hintY);
    glVertex2f(hintX + hintWidth, hintY + hintHeight);
    glVertex2f(hintX, hintY + hintHeight);
    glEnd();

    // Текст подсказки
    renderText(state, "Press 'H' for Help", hintX + 10, hintY + 25, 0.6f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Восстанавливаем матрицы
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Отрисовка клеточного поля
void drawGrid(float visibleLeft, float visibleRight,
    float visibleBottom, float visibleTop, float zoom, int fieldSize) {
    // Размер клетки в мировых координатах
    float cellSize = 2.0f / (10.0f * zoom);

    // Определяем границы видимой области в клетках
    int startX = (int)(visibleLeft / cellSize) - 1;
    int endX = (int)(visibleRight / cellSize) + 1;
    int startY = (int)(visibleBottom / cellSize) - 1;
    int endY = (int)(visibleTop / cellSize) + 1;

    // Если поле ограничено, корректируем границы отрисовки
    if (fieldSize > 0) {
        startX = (startX < -fieldSize / 2) ? -fieldSize / 2 : startX;
        endX = (endX > fieldSize / 2) ? fieldSize / 2 : endX;
        startY = (startY < -fieldSize / 2) ? -fieldSize / 2 : startY;
        endY = (endY > fieldSize / 2) ? fieldSize / 2 : endY;
    }

    // Отрисовываем только видимые клетки
    for (int x = startX; x <= endX; ++x) {
        for (int y = startY; y <= endY; ++y) {
            // Если поле ограничено, пропускаем клетки за границами
            if (fieldSize > 0 && (abs(x) > fieldSize / 2 || abs(y) > fieldSize / 2)) {
                continue;
            }

            // Вычисляем координаты углов клетки
            float x1 = x * cellSize;
            float y1 = y * cellSize;
            float x2 = x1 + cellSize;
            float y2 = y1 + cellSize;

            glLineWidth(3.0);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x1, y1);
            glVertex2f(x2, y1);
            glVertex2f(x2, y2);
            glVertex2f(x1, y2);
            glEnd();
        }
    }
}

// Отрисовка подсветки выбранной клетки
void drawSelectedCell(int x, int y, float cellSize) {
    // Вычисляем координаты углов выбранной клетки
    float x1 = x * cellSize;
    float y1 = y * cellSize;
    float x2 = x1 + cellSize;
    float y2 = y1 + cellSize;
    glColor3f(1.0, 1.0, 0.0);

    // Отрисовываем подсветку
    glLineWidth(5.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// Отрисовка крестика в клетке
void drawCross(float x1, float y1, float x2, float y2) {
    glColor3f(1.0, 0.0, 0.0);
    glLineWidth(10.0);
    glBegin(GL_LINES);
    glVertex2f((GLfloat)x1 + (GLfloat)0.03, (GLfloat)y1 + (GLfloat)0.03);
    glVertex2f((GLfloat)x2 - (GLfloat)0.03, (GLfloat)y2 - (GLfloat)0.03);
    glVertex2f((GLfloat)x1 + (GLfloat)0.03, (GLfloat)y2 - (GLfloat)0.03);
    glVertex2f((GLfloat)x2 - (GLfloat)0.03, (GLfloat)y1 + (GLfloat)0.03);
    glEnd();
}

// Отрисовка нолика в клетке
void drawCircle(float x1, float y1, float x2, float y2) {
    glColor3f(0.0, 0.0, 1.0);  // Синий цвет для нолика

    float centerX = (x1 + x2) / 2.0f;
    float centerY = (y1 + y2) / 2.0f;
    float outerRadius = (x2 - x1) / 2.5f;  // Внешний радиус
    float innerRadius = outerRadius * 0.7f; // Внутренний радиус для толщины линии

    // Рисуем нолик с помощью треугольников
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 360; i += 5) {  // Шаг 5 градусов можно уменьшить для большей гладкости
        float angle = i * 3.14159f / 180.0f;

        // Внешняя точка
        glVertex2f((GLfloat)centerX + (GLfloat)outerRadius * (GLfloat)cos(angle),
            (GLfloat)centerY + (GLfloat)outerRadius * (GLfloat)sin(angle));

        // Внутренняя точка
        glVertex2f((GLfloat)centerX + (GLfloat)innerRadius * (GLfloat)cos(angle),
            (GLfloat)centerY + (GLfloat)innerRadius * (GLfloat)sin(angle));
    }
    glEnd();
}

// Обработка зума колесом мыши
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (xoffset) {
        int kostil = 0;
        kostil++;
    }
    if (state->currentState != MENU_GAME) return;

    if (yoffset > 0)
        state->camera.zoom *= 1.1f;
    else if (yoffset < 0)
        state->camera.zoom /= 1.1f;

    // Ограничение зума
    if (state->camera.zoom < 0.4f) state->camera.zoom = 0.4f;
    if (state->camera.zoom > 3.0f) state->camera.zoom = 3.0f;
}


// Обработка нажатий клавиш мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS && state->currentState == MENU_SETTINGS) {
        handleSettingsClick(state, window, button);
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (state->currentState == MENU_MAIN) {
            // Обработка кликов и наведения в меню
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (mods == 1) {
                int kostil = 0;
                kostil++;
            }
            // Преобразуем координаты мыши в координаты экрана (1240x1240)
            ypos = height - ypos; // Инвертируем Y
            float menuX = ((float)xpos / (float)width) * 1240.0f;
            float menuY = ((float)ypos / (float)height) * 1240.0f;

            // Проверяем, какая кнопка под курсором
            if (menuX >= 500 && menuX <= 740) { // Ширина кнопки 240
                if (menuY >= 800 && menuY <= 880) { // New Game (первая кнопка)
                    state->menuSelectedItem = 0;
                    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                        cleanupGrid(&state->grid);
                        cleanupMoveLogger(&state->logger);
                        state->selectedCellX = 0;
                        state->selectedCellY = 0;
                        state->settings = state->defaultSettings;
                        state->currentState = MENU_GAME;
                        state->gameResult.rawResult = 0;
                        int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
                        
                        // Если бот ходит первым, делаем его ход сразу
                        if (state->settings.firstMove == FIRST_MOVE_AI) {
                            // Специальная логика для первого хода бота
                                addCell(&state->grid, 0, 0);
                                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                                logMove(&state->logger, 0, 0, MOVE_AI);
                        }

                    }
                }
                else if (menuY >= 600 && menuY <= 680) { // Load Game (вторая кнопка)
                    state->menuSelectedItem = 1;
                    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                        if (loadGame(&state->grid, &state->settings)) {
                            state->currentState = MENU_GAME;
                            state->selectedCellX = 0;
                            state->selectedCellY = 0;
                            state->gameResult.rawResult = 0;
                            
                            int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
                            int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;

                            

                            if (checkWinCondition(state, playerSymbol, state->settings.winLineLength)) {
                                state->gameResult.isWin = 1;
                            }
                            else if (checkWinCondition(state, aiSymbol, state->settings.winLineLength)) {
                                state->gameResult.isLose = 1;
                            }
                            else if (checkForDraw(state)) {
                                state->gameResult.isDraw = 1;
                            }
                        }
                    }
                }
                else if (menuY >= 400 && menuY <= 480) { // Settings (третья кнопка)
                    state->menuSelectedItem = 2;
                    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                        state->currentState = MENU_SETTINGS;
                    }
                }
                else if (menuY >= 200 && menuY <= 280) { // About (четвертая кнопка)
                    state->menuSelectedItem = 3;
                    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                        state->currentState = MENU_ABOUT;
                    }
                }
            }
        }
        else if (state->currentState == MENU_GAME) {
            state->mouse.isDragging = 1;
            glfwGetCursorPos(window, &state->mouse.lastX, &state->mouse.lastY);
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        state->mouse.isDragging = 0;
    }
    
}

// Обработка перемещения курсора
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (state->mouse.isDragging && state->currentState == MENU_GAME) {
        double deltaX = xpos - state->mouse.lastX;
        double deltaY = ypos - state->mouse.lastY;

        state->camera.offsetX -= (float)deltaX * 0.002f / state->camera.zoom;
        state->camera.offsetY += (float)deltaY * 0.002f / state->camera.zoom;

        state->mouse.lastX = xpos;
        state->mouse.lastY = ypos;
    }
    else if (state->currentState == MENU_MAIN) {
        // Обработка наведения в главном меню
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ypos = height - ypos; // Инвертируем Y
        float menuX = ((float)xpos / (float)width) * 1240.0f;
        float menuY = ((float)ypos / (float)height) * 1240.0f;

        // Проверяем, какая кнопка под курсором
        if (menuX >= 500 && menuX <= 740) { // Ширина кнопки 240
            if (menuY >= 800 && menuY <= 880) { // New Game (первая кнопка)
                
                state->menuSelectedItem = 0;

            }
            else if (menuY >= 600 && menuY <= 680) { // Load Game (вторая кнопка)
                state->menuSelectedItem = 1;
            }
            else if (menuY >= 400 && menuY <= 480) { // Settings (третья кнопка)
                state->menuSelectedItem = 2;
            }
            else if (menuY >= 200 && menuY <= 280) { // About (четвертая кнопка)
                state->menuSelectedItem = 3;
            }
        }
    }
}
// Проверка ничьи
int checkForDraw(AppState* state) {
    if (state->settings.fieldSize <= 0) return 0; // Бесконечное поле - ничья невозможна

    for (int x = -state->settings.fieldSize / 2; x <= state->settings.fieldSize / 2; x++) {
        for (int y = -state->settings.fieldSize / 2; y <= state->settings.fieldSize / 2; y++) {
            int cellEmpty = 1;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                    cellEmpty = 0;
                    break;
                }
            }
            if (cellEmpty) {
                return 0; // Нашли пустую клетку - ничьи нет
            }
        }
    }
    return 1; // Все клетки заполнены - ничья
}

// Обработка нажатия клавиш
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    if (scancode == 1) {
        int kostil = 0;
        kostil++;
    }

    if (state->currentState == MENU_MAIN) {
        switch (key) {
        case GLFW_KEY_UP:
            state->menuSelectedItem = (state->menuSelectedItem - 1 + 4) % 4; // По желанию в будущем замеить значения menuSelectedItem на enum
            break;
        case GLFW_KEY_DOWN:
            state->menuSelectedItem = (state->menuSelectedItem + 1) % 4;
            break;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_SPACE:
            switch (state->menuSelectedItem) {
            case 0: // New Game
                cleanupGrid(&state->grid);
                cleanupMoveLogger(&state->logger);
                state->selectedCellX = 0;
                state->selectedCellY = 0;
                state->camera.zoom = 1.0f;
                state->camera.offsetX = 0.0f;
                state->camera.offsetY = 0.0f;
                // Используем настройки из defaultSettings
                state->settings = state->defaultSettings;
                state->currentState = MENU_GAME;
                state->gameResult.rawResult = 0;

                // Если бот ходит первым, делаем его ход сразу
                if (state->settings.firstMove == FIRST_MOVE_AI) {
                    // Специальная логика для первого хода бота
                    if (state->settings.fieldSize > 0) {
                        // Для ограниченного поля - ставим в центр
                        addCell(&state->grid, 0, 0);
                        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                        logMove(&state->logger, 0, 0, MOVE_AI);
                    }
                    else {
                        // Для бесконечного поля - ставим в (0,0)
                        addCell(&state->grid, 0, 0);
                        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                        logMove(&state->logger, 0, 0, MOVE_AI);
                    }
                }
                break;
            case 1: // Load Game
                if (loadGame(&state->grid, &state->settings)) {
                    state->currentState = MENU_GAME;
                    state->selectedCellX = 0;
                    state->selectedCellY = 0;
                    state->camera.zoom = 1.0f;
                    state->camera.offsetX = 0.0f;
                    state->camera.offsetY = 0.0f;
                    state->gameResult.rawResult = 0; // Сбрасываем состояние игры
                    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
                    aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;

                    if (checkWinCondition(state, playerSymbol, state->settings.winLineLength)) {
                        state->gameResult.isWin = 1;
                    }
                    else if (checkWinCondition(state, aiSymbol, state->settings.winLineLength)) {
                        state->gameResult.isLose = 1;
                    }
                    else if (checkForDraw(state)) {
                        state->gameResult.isDraw = 1;
                    }
                }
                break;
            case 2: state->currentState = MENU_SETTINGS; break;
            case 3: state->currentState = MENU_ABOUT; break;
            }
            break;
        case GLFW_KEY_ESCAPE:
            if (state->currentState == MENU_GAME) {
                state->currentState = MENU_MAIN;
                state->gameResult.rawResult = 0; // Сбрасываем состояние игры при выходе
            }
            else {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            break;
        case GLFW_KEY_H:
            state->currentState = MENU_GAME;
            state->showHelp = 1;
            break;
        }
    }
    else if (state->currentState == MENU_GAME) {
        switch (key) {
        case GLFW_KEY_W:
        case GLFW_KEY_UP:
            state->selectedCellY += 1;
            if (state->settings.fieldSize > 0 &&
                state->selectedCellY > state->settings.fieldSize / 2) {
                state->selectedCellY = state->settings.fieldSize / 2;
            }
            break;
        case GLFW_KEY_S:
        case GLFW_KEY_DOWN:
            state->selectedCellY -= 1;
            if (state->settings.fieldSize > 0 &&
                state->selectedCellY < -state->settings.fieldSize / 2) {
                state->selectedCellY = -state->settings.fieldSize / 2;
            }
            break;
        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:
            state->selectedCellX -= 1;
            if (state->settings.fieldSize > 0 &&
                state->selectedCellX < -state->settings.fieldSize / 2) {
                state->selectedCellX = -state->settings.fieldSize / 2;
            }
            break;
        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT:
            state->selectedCellX += 1;
            if (state->settings.fieldSize > 0 &&
                state->selectedCellX > state->settings.fieldSize / 2) {
                state->selectedCellX = state->settings.fieldSize / 2;
            }
            break;
        case GLFW_KEY_LEFT_SHIFT: {
            float cellSize = 2.0f / (10.0f * state->camera.zoom);
            float cellCenterX = state->selectedCellX * cellSize + cellSize / 2.0f;
            float cellCenterY = state->selectedCellY * cellSize + cellSize / 2.0f;
            state->camera.offsetX = cellCenterX;
            state->camera.offsetY = cellCenterY;
            break;
        }
        case GLFW_KEY_SPACE: {
            if (state->gameResult.rawResult != 0) break; // Игра уже завершена

            // Определяем символ игрока и бота в зависимости от того, кто ходит первым
            int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
            aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;

            // Проверяем, не занята ли клетка
            int cellOccupied = 0;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == state->selectedCellX &&
                    state->grid.cells[i].y == state->selectedCellY &&
                    state->grid.cells[i].symbol != 0) {
                    cellOccupied = 1;
                    break;
                }
            }
            if (cellOccupied) break;

            // Делаем ход игрока
            int found = 0;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == state->selectedCellX &&
                    state->grid.cells[i].y == state->selectedCellY) {
                    state->grid.cells[i].symbol = playerSymbol;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                addCell(&state->grid, state->selectedCellX, state->selectedCellY);
                state->grid.cells[state->grid.size - 1].symbol = playerSymbol;
            }
            logMove(&state->logger, state->selectedCellX, state->selectedCellY, MOVE_PLAYER);

            // Проверка победы после хода
            if (checkWinCondition(state, playerSymbol, state->settings.winLineLength)) {
                state->gameResult.isWin = 1;
                break;
            }

            // Проверка ничьи
            if (checkForDraw(state)) {
                state->gameResult.isDraw = 1;
                break;
            }

            // Ход бота (только если игра не завершена)
            if (state->gameResult.rawResult == 0) {
                // Определяем функцию для хода бота в зависимости от сложности
                void (*aiMoveFunc)(AppState*) = NULL;
                switch (state->settings.difficulty) {
                case DIFFICULTY_EASY: aiMoveFunc = makeAIMoveEasy; break;
                case DIFFICULTY_MEDIUM: aiMoveFunc = makeAIMoveMedium; break;
                case DIFFICULTY_HARD: aiMoveFunc = makeAIMoveHard; break;
                case DIFFICULTY_EXPERT: aiMoveFunc = makeAIMoveExpert; break;
                }

                if (aiMoveFunc) {
                    aiMoveFunc(state);

                    // Проверка победы бота после его хода
                    if (checkWinCondition(state, aiSymbol, state->settings.winLineLength)) {
                        state->gameResult.isLose = 1;
                    }
                    // Проверка ничьи
                    else if (checkForDraw(state)) {
                        state->gameResult.isDraw = 1;
                    }
                }
            }
            break;
        }
        case GLFW_KEY_Q:
            if (mods & GLFW_MOD_ALT) {
                saveGame(&state->grid, &state->settings);
                state->saveNotificationTimer = 1.0; // Устанавливаем таймер на 2 секунды
                break;
            }
            break;
        case GLFW_KEY_ESCAPE:
            if (state->currentState == MENU_GAME) {
                // Восстанавливаем настройки из defaultSettings
                cleanupMoveLogger(&state->logger);
                state->settings = state->defaultSettings;
                state->currentState = MENU_MAIN;
                state->gameResult.rawResult = 0;
            }
            break;
        case GLFW_KEY_H:
            state->showHelp = !state->showHelp; // Переключаем отображение помощи
            break;
        case GLFW_KEY_M:  // M - показать/скрыть лог
            state->showMoveLog = !state->showMoveLog;
            break;


        }
    }
    else if (state->currentState == MENU_ABOUT || state->currentState == MENU_SETTINGS) {
        if (key == GLFW_KEY_ESCAPE) {
            state->currentState = MENU_MAIN;
        }
        else if (key == GLFW_KEY_H) {
            state->currentState = MENU_GAME;
            state->showHelp = 1;
        }
    }
   
}

// Отрисовка победнго сообщения
void drawWinLine(AppState* state, int width, int height) {
    if (state->gameResult.rawResult == 0) return;

    // Сохраняем матрицы
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Размеры и позиция уведомления
    float notifWidth = 400;
    float notifHeight = 100;
    float notifX = (width - notifWidth) / 2;
    float notifY = height - 150.0f;

    // Рисуем полупрозрачный фон
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (state->gameResult.isWin == 1) { // Победа
        glColor4f(0.1f, 0.5f, 0.1f, 0.9f);
    }
    else if (state->gameResult.isLose == 1) { // Поражение
        glColor4f(0.5f, 0.1f, 0.1f, 0.9f);
    }
    else { // Ничья
        glColor4f(0.3f, 0.3f, 0.3f, 0.9f);
    }

    glBegin(GL_QUADS);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();

    // Рамка
    glColor4f(0.8f, 0.8f, 0.0f, 0.9f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();

    // Текст уведомления
    if (state->gameResult.isWin == 1) {
        renderText(state, "You Win! Press ESC to return to menu",
            notifX + 20, notifY + 60,
            0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (state->gameResult.isLose == 1) {
        renderText(state, "You Lose! Press ESC to return to menu",
            notifX + 20, notifY + 60,
            0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
        renderText(state, "Draw! Press ESC to return to menu",
            notifX + 20, notifY + 60,
            0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Восстанавливаем матрицы
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Отрисовка победной линии
void drawWinningLine(AppState* state) {

    // Определяем, кто выиграл
    int winnerSymbol = 0;
    if (state->gameResult.isWin) {
        winnerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    }
    else if (state->gameResult.isLose) {
        winnerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    }

    // Устанавливаем цвет в зависимости от того, кто выиграл
    if (winnerSymbol == 1) { // Крестик
        int isPlayer = (state->settings.firstMove == FIRST_MOVE_PLAYER);
        if (isPlayer) {
            glColor3f(1.0f, 0.0f, 0.0f); // Красный для игрока
        }
        else {
            glColor3f(0.5f, 0.0f, 0.5f); // Фиолетовый для бота
        }
    }
    else { // Нолик
        int isPlayer = (state->settings.firstMove != FIRST_MOVE_PLAYER);
        if (isPlayer) {
            glColor3f(0.0f, 0.0f, 1.0f); // Синий для игрока
        }
        else {
            glColor3f(0.0f, 1.0f, 1.0f); // Голубой для бота
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Рисуем линию между центрами клеток
    glColor3f(1.0f, 1.0f, 0.0f); // Желтый цвет
    glLineWidth(5.0f);
    glBegin(GL_LINES);

    float cellSize = 2.0f / (10.0f * state->camera.zoom);
    float x1 = state->winLineStartX * cellSize + cellSize / 2;
    float y1 = state->winLineStartY * cellSize + cellSize / 2;
    float x2 = state->winLineEndX * cellSize + cellSize / 2;
    float y2 = state->winLineEndY * cellSize + cellSize / 2;

    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();

    glPopMatrix();
}







                                                    /*                         АЛГОРИТМЫ                                  */


// Проверка, есть ли выигрышная линия из symbol заданной длины
int checkWinCondition(AppState* state, int symbol, int winLength) {
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol != symbol) continue;

        int x = state->grid.cells[i].x;
        int y = state->grid.cells[i].y;

        int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

        for (int d = 0; d < 4; d++) {
            int dx = directions[d][0];
            int dy = directions[d][1];
            int count = 1;
            int startX = x, startY = y;
            int endX = x, endY = y;

            // Проверяем в одном направлении
            for (int step = 1; step < winLength; step++) {
                int found = 0;
                for (int j = 0; j < state->grid.size; j++) {
                    if (state->grid.cells[j].x == x + dx * step &&
                        state->grid.cells[j].y == y + dy * step &&
                        state->grid.cells[j].symbol == symbol) {
                        found = 1;
                        endX = x + dx * step;
                        endY = y + dy * step;
                        break;
                    }
                }
                if (!found) break;
                count++;
            }

            // Проверяем в противоположном направлении
            for (int step = 1; step < winLength; step++) {
                int found = 0;
                for (int j = 0; j < state->grid.size; j++) {
                    if (state->grid.cells[j].x == x - dx * step &&
                        state->grid.cells[j].y == y - dy * step &&
                        state->grid.cells[j].symbol == symbol) {
                        found = 1;
                        startX = x - dx * step;
                        startY = y - dy * step;
                        break;
                    }
                }
                if (!found) break;
                count++;
            }

            if (count >= winLength) {
                state->winLineStartX = startX;
                state->winLineStartY = startY;
                state->winLineEndX = endX;
                state->winLineEndY = endY;
                return 1;
            }
        }
    }
    return 0;
}



// Функция для хода бота (легкий уровень), условно рандом в радиусе длины победной линии
void makeAIMoveEasy(AppState* state) {
    
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }


    // Собираем все игрока
    int crossCount = 0;
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) crossCount++;
    }

    

    // Выбираем случайный
    int randomCrossIndex = rand() % crossCount;
    int crossFound = 0;
    int targetX = 0, targetY = 0;

    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) {
            if (crossFound == randomCrossIndex) {
                targetX = state->grid.cells[i].x;
                targetY = state->grid.cells[i].y;
                break;
            }
            crossFound++;
        }
    }

    // Пытаемся поставить рядом с выбранным крестиком
    int attempts = 0;
    const int maxAttempts = state->settings.winLineLength;

    while (attempts < maxAttempts) {
        // Выбираем случайное направление и расстояние (не больше winLineLength)
        int dx = (rand() % (2 * state->settings.winLineLength + 1)) - state->settings.winLineLength;
        int dy = (rand() % (2 * state->settings.winLineLength + 1)) - state->settings.winLineLength;

        // Убедимся, что мы не остались на месте
        if (dx == 0 && dy == 0) continue;

        int newX = targetX + dx;
        int newY = targetY + dy;

        // Проверяем, что клетка свободна и в пределах поля (если поле ограничено)
        int cellFree = 1;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].x == newX &&
                state->grid.cells[i].y == newY) {
                cellFree = 0;
                break;
            }
        }

        if (cellFree) {
            if (state->settings.fieldSize > 0) {
                // Проверяем, что в пределах ограниченного поля
                if (abs(newX) <= state->settings.fieldSize / 2 &&
                    abs(newY) <= state->settings.fieldSize / 2) {
                    addCell(&state->grid, newX, newY);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                    logMove(&state->logger, newX, newY, MOVE_AI);
                    return;
                }
            }
            else {
                // Для бесконечного поля просто добавляем
                addCell(&state->grid, newX, newY);
                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                logMove(&state->logger, newX, newY, MOVE_AI);
                return;
            }
        }

        attempts++;
    }

    // Если не удалось найти место рядом, ставим в случайную свободную клетку
    if (state->settings.fieldSize > 0) {
        // Для ограниченного поля
        for (int attempt = 0; attempt < 100; attempt++) {
            int x = (rand() % state->settings.fieldSize) - state->settings.fieldSize / 2;
            int y = (rand() % state->settings.fieldSize) - state->settings.fieldSize / 2;

            int cellFree = 1;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == x &&
                    state->grid.cells[i].y == y) {
                    cellFree = 0;
                    break;
                }
            }

            if (cellFree) {
                addCell(&state->grid, x, y);
                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                logMove(&state->logger, x, y, MOVE_AI);
                return;
            }
        }
    }
    else {
        for (int attempt = 0; attempt < 100; attempt++) {
            int x = (rand() % 21) - state->settings.winLineLength;
            int y = (rand() % 21) - state->settings.winLineLength;

            int cellFree = 1;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == x &&
                    state->grid.cells[i].y == y) {
                    cellFree = 0;
                    break;
                }
            }

            if (cellFree) {
                addCell(&state->grid, x, y);
                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                logMove(&state->logger, x, y, MOVE_AI);
                return;
            }
        }
    }
}

// Бот средней сложности меньше рандома, видит очевидные угрозы
void makeAIMoveMedium(AppState* state) {
    

    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. Проверить, есть ли выигрышный ход для бота (нолика)
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Ищем свои нолики
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // Проверяем все направления
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один нолик
                    int emptyCount = 0;
                    int emptyX = -1, emptyY = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == aiSymbol) {
                                    count++;
                                }
                                else if (state->grid.cells[j].symbol == playerSymbol) {
                                    found = -1; // Крестик мешает
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Крестик на пути
                        }
                    }

                    // Если нашли выигрышный ход (ровно одна пустая клетка в линии нужной длины)
                    if (count == state->settings.winLineLength - 1 && emptyCount == 1) {
                        // Проверяем, что клетка в пределах поля
                        if (state->settings.fieldSize == 0 ||
                            (abs(emptyX) <= state->settings.fieldSize / 2 &&
                                abs(emptyY) <= state->settings.fieldSize / 2)) {
                            addCell(&state->grid, emptyX, emptyY);
                            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                            logMove(&state->logger, emptyX, emptyY, MOVE_AI);
                            return;
                        }
                    }
                }
            }
        }
    }

    // 2. Проверить, нужно ли блокировать выигрышный ход игрока
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Ищем крестики игрока
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один крестик
                    int emptyCount = 0;
                    int emptyX = -1, emptyY = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == playerSymbol) {
                                    count++;
                                }
                                else if (state->grid.cells[j].symbol == aiSymbol) {
                                    found = -1; // Нолик мешает
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Нолик на пути
                        }
                    }

                    // Если нужно блокировать (ровно одна пустая клетка в линии нужной длины)
                    if (count == state->settings.winLineLength - 1 && emptyCount == 1) {
                        if (state->settings.fieldSize == 0 ||
                            (abs(emptyX) <= state->settings.fieldSize / 2 &&
                                abs(emptyY) <= state->settings.fieldSize / 2)) {
                            addCell(&state->grid, emptyX, emptyY);
                            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                            logMove(&state->logger, emptyX, emptyY, MOVE_AI);
                            return;
                        }
                    }
                }
            }
        }
    }

    // 3. Занять центр, если он свободен
    if (state->settings.fieldSize > 0) {
        int centerX = 0, centerY = 0;
        int centerFree = 1;

        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].x == centerX &&
                state->grid.cells[i].y == centerY) {
                centerFree = 0;
                break;
            }
        }

        if (centerFree) {
            addCell(&state->grid, centerX, centerY);
            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
            logMove(&state->logger, centerX, centerY, MOVE_AI);
            return;
        }
    }

    // 4. Случайный выбор между атакой и защитой (упрощенная версия)
    int strategy = rand() % 2; // 0 - атака, 1 - защита

    if (strategy == 0) { // Атака - поставить рядом со своим ноликом
        // Собираем все свои нолики
        int nolikCount = 0;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].symbol == aiSymbol) nolikCount++;
        }

        if (nolikCount > 0) {
            // Выбираем случайный нолик
            int randomIndex = rand() % nolikCount;
            int found = 0;
            int targetX = 0, targetY = 0;

            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].symbol == aiSymbol) {
                    if (found == randomIndex) {
                        targetX = state->grid.cells[i].x;
                        targetY = state->grid.cells[i].y;
                        break;
                    }
                    found++;
                }
            }

            // Пытаемся поставить рядом (в радиусе 1 клетки)
            for (int attempt = 0; attempt < 8; attempt++) {
                int dx = (rand() % 3) - 1; // -1, 0 или 1
                int dy = (rand() % 3) - 1;

                if (dx == 0 && dy == 0) continue;

                int newX = targetX + dx;
                int newY = targetY + dy;

                // Проверяем, что клетка свободна
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == newX &&
                        state->grid.cells[i].y == newY) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Проверяем границы поля
                    if (state->settings.fieldSize > 0) {
                        if (abs(newX) > state->settings.fieldSize / 2 ||
                            abs(newY) > state->settings.fieldSize / 2) {
                            continue;
                        }
                    }

                    addCell(&state->grid, newX, newY);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                    logMove(&state->logger, newX, newY, MOVE_AI);
                    return;
                }
            }
        }
    }
    else { // Защита - поставить рядом с крестиком
        // Собираем все крестики
        int krestikCount = 0;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].symbol == playerSymbol) krestikCount++;
        }

        if (krestikCount > 0) {
            // Выбираем случайный крестик
            int randomIndex = rand() % krestikCount;
            int found = 0;
            int targetX = 0, targetY = 0;

            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].symbol == playerSymbol) {
                    if (found == randomIndex) {
                        targetX = state->grid.cells[i].x;
                        targetY = state->grid.cells[i].y;
                        break;
                    }
                    found++;
                }
            }

            // Пытаемся поставить рядом (в радиусе 1 клетки)
            for (int attempt = 0; attempt < 8; attempt++) {
                int dx = (rand() % 3) - 1;
                int dy = (rand() % 3) - 1;

                if (dx == 0 && dy == 0) continue;

                int newX = targetX + dx;
                int newY = targetY + dy;

                // Проверяем, что клетка свободна
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == newX &&
                        state->grid.cells[i].y == newY) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Проверяем границы поля
                    if (state->settings.fieldSize > 0) {
                        if (abs(newX) > state->settings.fieldSize / 2 ||
                            abs(newY) > state->settings.fieldSize / 2) {
                            continue;
                        }
                    }

                    addCell(&state->grid, newX, newY);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                    logMove(&state->logger, newX, newY, MOVE_AI);
                    return;
                }
            }
        }
    }

    // Если все стратегии не сработали, делаем случайный ход
    makeAIMoveEasy(state);
}


// Бот сложного уровня, работает на эвристиках, довольно эффективен, подробнее в отчете
void makeAIMoveHard(AppState* state) {
    

    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. Проверить, есть ли выигрышный ход для бота (нолика)
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Ищем свои нолики
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // Проверяем все направления
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один нолик
                    int emptyCount = 0;
                    int emptyX = -1, emptyY = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == aiSymbol) {
                                    count++;
                                }
                                else if (state->grid.cells[j].symbol == playerSymbol) {
                                    found = -1; // Крестик мешает
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Крестик на пути
                        }
                    }

                    // Если нашли выигрышный ход (ровно одна пустая клетка в линии нужной длины)
                    if (count >= state->settings.winLineLength - 1 && emptyCount == 1) {
                        // Проверяем, что клетка в пределах поля
                        if (state->settings.fieldSize == 0 ||
                            (abs(emptyX) <= state->settings.fieldSize / 2 &&
                                abs(emptyY) <= state->settings.fieldSize / 2)) {
                            addCell(&state->grid, emptyX, emptyY);
                            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                            logMove(&state->logger, emptyX, emptyY, MOVE_AI);
                            return;
                        }
                    }
                }
            }
        }
    }

    // 2. Проверить, нужно ли блокировать выигрышный ход игрока
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Ищем крестики игрока
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один крестик
                    int emptyCount = 0;
                    int emptyX = -1, emptyY = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == playerSymbol) {
                                    count++;
                                }
                                else if (state->grid.cells[j].symbol == aiSymbol) {
                                    found = -1; // Нолик мешает
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Нолик на пути
                        }
                    }

                    // Если нужно блокировать (линия из winLength-1 крестиков с одной пустой)
                    if (count >= state->settings.winLineLength - 1 && emptyCount == 1) {
                        if (state->settings.fieldSize == 0 ||
                            (abs(emptyX) <= state->settings.fieldSize / 2 &&
                                abs(emptyY) <= state->settings.fieldSize / 2)) {
                            addCell(&state->grid, emptyX, emptyY);
                            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                            logMove(&state->logger, emptyX, emptyY, MOVE_AI);
                            return;
                        }
                    }
                }
            }
        }
    }
    // 4. Занять центр, если он свободен (только для ограниченного поля)
    if (state->settings.fieldSize > 0) {
        int centerX = 0, centerY = 0;
        int centerFree = 1;

        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].x == centerX &&
                state->grid.cells[i].y == centerY) {
                centerFree = 0;
                break;
            }
        }

        if (centerFree) {
            addCell(&state->grid, centerX, centerY);
            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
            logMove(&state->logger, centerX, centerY, MOVE_AI);
            return;
        }
    }

    // 3. Поиск стратегически важных позиций (длинные линии)
    // Сначала ищем свои линии, которые можно продолжить
    int bestScore = -1;
    int bestX = 0, bestY = 0;

    // Проверяем все возможные пустые клетки
    for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
        for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
            // Для ограниченного поля пропускаем клетки за границами
            if (state->settings.fieldSize > 0 &&
                (abs(x) > state->settings.fieldSize / 2 ||
                    abs(y) > state->settings.fieldSize / 2)) {
                continue;
            }

            // Проверяем, что клетка свободна
            int cellFree = 1;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                    cellFree = 0;
                    break;
                }
            }

            if (!cellFree) continue;

            // Оцениваем клетку по всем направлениям
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };
            int cellScore = 0;

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                int myCount = 0;   // Нолики
                int oppCount = 0;   // Крестики
                int emptyCount = 0;  // Пустые

                for (int step = -state->settings.winLineLength + 1;
                    step < state->settings.winLineLength; step++) {
                    if (step == 0) continue; // Пропускаем центральную клетку (она пустая)

                    int currentX = x + dx * step;
                    int currentY = y + dy * step;

                    // Для ограниченного поля проверяем границы
                    if (state->settings.fieldSize > 0 &&
                        (abs(currentX) > state->settings.fieldSize / 2 ||
                            abs(currentY) > state->settings.fieldSize / 2)) {
                        continue;
                    }

                    // Проверяем содержимое клетки
                    int found = 0;
                    for (int i = 0; i < state->grid.size; i++) {
                        if (state->grid.cells[i].x == currentX &&
                            state->grid.cells[i].y == currentY) {
                            if (state->grid.cells[i].symbol == aiSymbol) myCount++;
                            else if (state->grid.cells[i].symbol == playerSymbol) oppCount++;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) emptyCount++;
                }

                // Оцениваем потенциал линии
                if (myCount > 0 && oppCount == 0) {
                    // Линия с нашими символами - хорошая возможность
                    cellScore += myCount * myCount;
                }
                else if (oppCount > 0 && myCount == 0) {
                    // Линия с символами противника - нужно блокировать
                    cellScore += oppCount * oppCount;
                }
            }

            // Обновляем лучшую клетку
            if (cellScore > bestScore) {
                bestScore = cellScore;
                bestX = x;
                bestY = y;
            }
        }
    }

    // Если нашли хорошую позицию, делаем ход
    if (bestScore > 0) {
        addCell(&state->grid, bestX, bestY);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, bestX, bestY, MOVE_AI);
        return;
    }

    

    // 5. Если ничего стратегического не найдено, делаем ход рядом с существующими ноликами
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Нашли нолик
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // Проверяем все соседние клетки
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;

                    int newX = x + dx;
                    int newY = y + dy;

                    // Проверяем границы для ограниченного поля
                    if (state->settings.fieldSize > 0 &&
                        (abs(newX) > state->settings.fieldSize / 2 ||
                            abs(newY) > state->settings.fieldSize / 2)) {
                        continue;
                    }

                    // Проверяем, что клетка свободна
                    int cellFree = 1;
                    for (int j = 0; j < state->grid.size; j++) {
                        if (state->grid.cells[j].x == newX &&
                            state->grid.cells[j].y == newY) {
                            cellFree = 0;
                            break;
                        }
                    }

                    if (cellFree) {
                        addCell(&state->grid, newX, newY);
                        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                        logMove(&state->logger, newX, newY, MOVE_AI);
                        return;
                    }
                }
            }
        }
    }

    // 6. Если все остальное не сработало, делаем случайный ход
    makeAIMoveMedium(state);
}


// Вспомогательная функция для оценки позиции (для минимакса)
int evaluatePosition(AppState* state, int symbol, int opponentSymbol) {
    int score = 0;
    

    // Проверяем все клетки с нашим символом
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol != symbol) continue;

        int x = state->grid.cells[i].x;
        int y = state->grid.cells[i].y;

        int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

        for (int d = 0; d < 4; d++) {
            int dx = directions[d][0];
            int dy = directions[d][1];

            // Проверяем линию в обоих направлениях
            int count = 1; // Уже есть один наш символ
            int openEnds = 0;
            int blockedEnds = 0;

            // Проверяем в одном направлении
            for (int step = 1; step < state->settings.winLineLength; step++) {
                int currentX = x + dx * step;
                int currentY = y + dy * step;

                int found = 0;
                for (int j = 0; j < state->grid.size; j++) {
                    if (state->grid.cells[j].x == currentX &&
                        state->grid.cells[j].y == currentY) {
                        if (state->grid.cells[j].symbol == symbol) count++;
                        else if (state->grid.cells[j].symbol == opponentSymbol) blockedEnds++;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    // Проверяем границы для ограниченного поля
                    if (state->settings.fieldSize > 0 &&
                        (abs(currentX) > state->settings.fieldSize / 2 ||
                            abs(currentY) > state->settings.fieldSize / 2)) {
                        blockedEnds++;
                    }
                    else {
                        openEnds++;
                    }
                    break;
                }
            }

            // Проверяем в противоположном направлении
            for (int step = 1; step < state->settings.winLineLength; step++) {
                int currentX = x - dx * step;
                int currentY = y - dy * step;

                int found = 0;
                for (int j = 0; j < state->grid.size; j++) {
                    if (state->grid.cells[j].x == currentX &&
                        state->grid.cells[j].y == currentY) {
                        if (state->grid.cells[j].symbol == symbol) count++;
                        else if (state->grid.cells[j].symbol == opponentSymbol) blockedEnds++;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    // Проверяем границы для ограниченного поля
                    if (state->settings.fieldSize > 0 &&
                        (abs(currentX) > state->settings.fieldSize / 2 ||
                            abs(currentY) > state->settings.fieldSize / 2)) {
                        blockedEnds++;
                    }
                    else {
                        openEnds++;
                    }
                    break;
                }
            }

            // Оцениваем потенциальную линию
            if (count >= state->settings.winLineLength) {
                return (symbol == 2) ? 10000 : -10000; // Победа/поражение
            }

            if (blockedEnds == 0) {
                if (count == state->settings.winLineLength - 1) score += (symbol == 2) ? 5000 : -5000;
                else if (count == state->settings.winLineLength - 2) score += (symbol == 2) ? 1000 : -1000;
                else if (count >= 2) score += (symbol == 2) ? count * count * 10 : -count * count * 10;
            }
            else if (blockedEnds == 1) {
                if (count == state->settings.winLineLength - 1) score += (symbol == 2) ? 1000 : -1000;
                else if (count >= 2) score += (symbol == 2) ? count * count : -count * count;
            }
        }
    }

    return score;
}

// Минимакс с альфа-бета отсечением
int minimax(AppState* state, int depth, int isMaximizing, int alpha, int beta, int* bestX, int* bestY, int aiSymbol, int playerSymbol) {
    // Проверяем терминальные состояния
    int playerWin = checkWinCondition(state, playerSymbol, state->settings.winLineLength);
    int aiWin = checkWinCondition(state, aiSymbol, state->settings.winLineLength);
    int draw = checkForDraw(state);

    if (aiWin) return 100000 - depth; // Чем глубже, тем меньше очки
    if (playerWin) return -100000 + depth;
    if (draw) return 0;
    if (depth == 0) return evaluatePosition(state, playerSymbol, aiSymbol);

    if (isMaximizing) {
        int maxEval = -1000000;

        // Генерируем возможные ходы
        for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
            for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
                // Проверяем границы для ограниченного поля
                if (state->settings.fieldSize > 0 &&
                    (abs(x) > state->settings.fieldSize / 2 ||
                        abs(y) > state->settings.fieldSize / 2)) {
                    continue;
                }

                // Проверяем, что клетка свободна
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Делаем ход
                    addCell(&state->grid, x, y);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;

                    int eval = minimax(state, depth - 1, 0, alpha, beta, NULL, NULL, aiSymbol, playerSymbol);

                    // Отменяем ход
                    state->grid.size--;

                    if (eval > maxEval) {
                        maxEval = eval;
                        if (bestX && bestY) {
                            *bestX = x;
                            *bestY = y;
                        }
                    }

                    alpha = (alpha > eval) ? alpha : eval;
                    if (beta <= alpha) break;
                }
            }
        }
        return maxEval;
    }
    else {
        int minEval = 1000000;

        // Генерируем возможные ходы
        for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
            for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
                // Проверяем границы для ограниченного поля
                if (state->settings.fieldSize > 0 &&
                    (abs(x) > state->settings.fieldSize / 2 ||
                        abs(y) > state->settings.fieldSize / 2)) {
                    continue;
                }

                // Проверяем, что клетка свободна
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Делаем ход
                    addCell(&state->grid, x, y);
                    state->grid.cells[state->grid.size - 1].symbol = playerSymbol;

                    int eval = minimax(state, depth - 1, 1, alpha, beta, NULL, NULL, aiSymbol, playerSymbol);

                    // Отменяем ход
                    state->grid.size--;

                    if (eval < minEval) {
                        minEval = eval;
                    }

                    beta = (beta < eval) ? beta : eval;
                    if (beta <= alpha) break;
                }
            }
        }
        return minEval;
    }
}


// Основная функция для экспертного уровня, реализация частично через эвристики и через минимакса с альфа бета отсечением, довольно хорош, но не эффективен на больших линиях и полях
void makeAIMoveExpert(AppState* state) {
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;

    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. Проверить, есть ли выигрышный ход для бота (нолика)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Ищем свои нолики
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один нолик
                    int emptyCount = 0;
                    int emptyX = -1, emptyY = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == aiSymbol) count++;
                                else if (state->grid.cells[j].symbol == 1) found = -1;
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;
                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) break;
                    }

                    // Выигрышный ход (не хватает одной фигуры)
                    if (count >= state->settings.winLineLength - 1 && emptyCount == 1) {
                        if (state->settings.fieldSize == 0 ||
                            (abs(emptyX) <= state->settings.fieldSize / 2 &&
                                abs(emptyY) <= state->settings.fieldSize / 2)) {
                            addCell(&state->grid, emptyX, emptyY);
                            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                            logMove(&state->logger, emptyX, emptyY, MOVE_AI);
                            return;
                        }
                    }
                }
            }
        }
    }

    // 2. Проверить, нужно ли блокировать выигрышный ход игрока (не хватает одной фигуры)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Ищем крестики игрока
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один крестик
                    int emptyCount = 0;
                    int emptyX = -1, emptyY = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == playerSymbol) count++;
                                else if (state->grid.cells[j].symbol == aiSymbol) found = -1;
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;
                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) break;
                    }

                    // Срочная блокировка (не хватает одной фигуры)
                    if (count >= state->settings.winLineLength - 1 && emptyCount == 1) {
                        if (state->settings.fieldSize == 0 ||
                            (abs(emptyX) <= state->settings.fieldSize / 2 &&
                                abs(emptyY) <= state->settings.fieldSize / 2)) {
                            addCell(&state->grid, emptyX, emptyY);
                            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                            logMove(&state->logger, emptyX, emptyY, MOVE_AI);
                            return;
                        }
                    }
                }
            }
        }
    }

    // 3. Проверить опасные ситуации (не хватает двух фигур)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Ищем крестики игрока
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Проверяем линию в обоих направлениях
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Уже есть один крестик
                    int emptyCount = 0;
                    int emptyX1 = -1, emptyY1 = -1;
                    int emptyX2 = -1, emptyY2 = -1;

                    for (int step = 1; step < state->settings.winLineLength; step++) {
                        int currentX = x + dx * step * dir;
                        int currentY = y + dy * step * dir;

                        int found = 0;
                        for (int j = 0; j < state->grid.size; j++) {
                            if (state->grid.cells[j].x == currentX &&
                                state->grid.cells[j].y == currentY) {
                                if (state->grid.cells[j].symbol == playerSymbol) count++;
                                else if (state->grid.cells[j].symbol == aiSymbol) found = -1;
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Пустая клетка
                            if (emptyCount == 0) {
                                emptyX1 = currentX;
                                emptyY1 = currentY;
                            }
                            else {
                                emptyX2 = currentX;
                                emptyY2 = currentY;
                            }
                            emptyCount++;
                            if (emptyCount > 2) break;
                        }
                        else if (found == -1) break;
                    }

                    // Опасная ситуация (не хватает двух фигур)
                    if (count >= state->settings.winLineLength - 2 && emptyCount >= 2) {
                        // Предпочитаем блокировать клетку, которая создаст больше возможностей для нас
                        if (emptyX1 != -1 && emptyY1 != -1) {
                            if (state->settings.fieldSize == 0 ||
                                (abs(emptyX1) <= state->settings.fieldSize / 2 &&
                                    abs(emptyY1) <= state->settings.fieldSize / 2)) {
                                addCell(&state->grid, emptyX1, emptyY1);
                                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                                logMove(&state->logger, emptyX1, emptyY1, MOVE_AI);
                                return;
                            }
                        }
                        if (emptyX2 != -1 && emptyY2 != -1) {
                            if (state->settings.fieldSize == 0 ||
                                (abs(emptyX2) <= state->settings.fieldSize / 2 &&
                                    abs(emptyY2) <= state->settings.fieldSize / 2)) {
                                addCell(&state->grid, emptyX2, emptyY2);
                                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                                logMove(&state->logger, emptyX2, emptyY2, MOVE_AI);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    // 4. Использовать минимакс для стратегических ходов
    int bestX = 0, bestY = 0;
    if (state->settings.fieldSize == 3) {
        minimax(state, 100, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);
    }
    else if (state->settings.fieldSize == 0 || state->settings.fieldSize > 15) {
        if (state->settings.winLineLength >= 8) {
            minimax(state, 2, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);

        }
        else if (state->settings.winLineLength > 6) {
            minimax(state, 3, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);
        }
        else if (state->settings.winLineLength >= 4) {
            minimax(state, 3, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);
        }
    }
    else if (state->settings.fieldSize > 0) {
        if (state->settings.winLineLength > 10) {
            minimax(state, 3, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);

        }
        else if (state->settings.winLineLength > 7) {
            minimax(state, 3, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);
        }
        else if (state->settings.winLineLength >= 4) {
            minimax(state, 3, 1, -1000000, 1000000, &bestX, &bestY, aiSymbol, playerSymbol);
        }
    }
    
    if (bestX != 0 || bestY != 0) {
        int cellFree = 1;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].x == bestX && state->grid.cells[i].y == bestY) {
                cellFree = 0;
                break;
            }
        }

        if (cellFree && (state->settings.fieldSize == 0 ||
            (abs(bestX) <= state->settings.fieldSize / 2 &&
                abs(bestY) <= state->settings.fieldSize / 2))) {
            addCell(&state->grid, bestX, bestY);
            state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
            logMove(&state->logger, bestX, bestY, MOVE_AI);
            return;
        }
    }

    // 5. Если все остальное не сработало, используем стратегию из сложного уровня
    makeAIMoveHard(state);
}



int main() {
    
    
    
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }

    // Создаем окно
    GLFWwindow* window = glfwCreateWindow(1240, 1240, "Krestiki-Noliki", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Ошибка создания окна\n");
        glfwTerminate();
        return -1;
    }

    // Делаем контекст текущим
    glfwMakeContextCurrent(window);

    // Инициализация состояния приложения
    AppState state;
    initAppState(&state);
    

    // Инициализация текста
    if (!initText(&state, "text/arial.ttf")) {
        fprintf(stderr, "Не удалось загрузить шрифт\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowUserPointer(window, &state);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Основной цикл рендеринга
    while (!glfwWindowShouldClose(window)) {
        // Очистка буфера
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Получаем размеры окна
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (state.saveNotificationTimer > 0.0f) {
            state.saveNotificationTimer -= 0.005f; // Уменьшаем на время кадра 
        }
        


        // Выбираем что отрисовывать в зависимости от состояния
        switch (state.currentState) {
        case MENU_MAIN:
            drawMainMenu(&state);
            break;

        case MENU_GAME: {
            // Устанавливаем матрицу проекции
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            // Рассчитываем видимую область
            float aspectRatio = (float)width / (float)height;
            float visibleLeft = -1.0f * state.camera.zoom * aspectRatio + state.camera.offsetX;
            float visibleRight = 1.0f * state.camera.zoom * aspectRatio + state.camera.offsetX;
            float visibleBottom = -1.0f * state.camera.zoom + state.camera.offsetY;
            float visibleTop = 1.0f * state.camera.zoom + state.camera.offsetY;

            glOrtho(visibleLeft, visibleRight, visibleBottom, visibleTop, -1.0, 1.0);

            // Устанавливаем матрицу модели
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glColor3f(0.0, 0.0, 0.0);

            // Отрисовываем поле
            drawGrid(visibleLeft, visibleRight, visibleBottom, visibleTop, state.camera.zoom, state.settings.fieldSize);

            // Отрисовываем выбранную клетку
            float cellSize = 2.0f / (10.0f * state.camera.zoom);
            drawSelectedCell(state.selectedCellX, state.selectedCellY, cellSize);

            // Отрисовываем символы
            for (int i = 0; i < state.grid.size; i++) {
                float x1 = state.grid.cells[i].x * cellSize;
                float y1 = state.grid.cells[i].y * cellSize;
                float x2 = x1 + cellSize;
                float y2 = y1 + cellSize;
                
                if (state.grid.cells[i].symbol == 1) { // Крестик 
                    drawCross(x1, y1, x2, y2);
                }
                else if (state.grid.cells[i].symbol == 2) { // Нолик
                    drawCircle(x1, y1, x2, y2);
                }
            }
            if (state.currentState == MENU_SETTINGS) {
                drawSettingsScreen(&state, window);
        }
            if (state.saveNotificationTimer > 0.0f) { // таймер уведы о сейве
                drawSaveNotification(&state, width, height);
            }
            if (state.gameResult.rawResult  != 0) { // победная линия
                drawWinLine(&state, width, height);
                if (state.gameResult.isDraw != 1) {
                    drawWinningLine(&state);
                }
            }

            // Рисуем подсказку о помощи (только если сама помощь не отображается)
            if (!state.showHelp) {
                drawHelpHint(&state, width, height);
            }

            if (state.showMoveLog) {
                drawMoveLog(&state);
            }

            // Если нужно показать помощь, рисуем поверх всего
            if (state.showHelp) {
                drawHelpWindow(&state);
            }
            break;
        }

        case MENU_ABOUT:
            drawAboutScreen(&state);
            break;

        case MENU_SETTINGS:
            drawSettingsScreen(&state, window);
            break;
        }

        // Меняем буферы и обрабатываем события
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Освобождаем ресурсы
    glDeleteTextures(1, &state.fontTexture);
    cleanupGrid(&state.grid);
    cleanupMoveLogger(&state.logger);
    glfwTerminate();
    
    return 0;
}