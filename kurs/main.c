#define _CRT_SECURE_NO_WARNINGS
#include "include/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define STB_TRUETYPE_IMPLEMENTATION
#include "text/stb_truetype.h"
// ���������� ��� ��������� ������� � �������, ������������� ��������� ����� arial.ttf



                                                    /*                                ���������                                     */
// ��������� ���� ����������
typedef enum {
    MENU_MAIN,     // ������� ����
    MENU_GAME,     // ������� �����
    MENU_ABOUT,    // ����� "� ���������"
    MENU_SETTINGS  // ����� ��������
} AppMenuState;

// ������ ��������� ����
typedef enum {
    DIFFICULTY_EASY,    // ������
    DIFFICULTY_MEDIUM,  // �������
    DIFFICULTY_HARD,    // �������
    DIFFICULTY_EXPERT   // �������
} GameDifficulty;

typedef enum {
    GAME_RESULT_NONE = 0,   // ���� ������������
    GAME_RESULT_WIN = 1,    // ������
    GAME_RESULT_LOSE = -1,  // ���������
    GAME_RESULT_DRAW = 2    // �����
} GameResultType;

// ��� ���� (�����/���)
typedef enum {
    MOVE_PLAYER,  // ��� ������ (�������)
    MOVE_AI       // ��� ���� (�����)
} MoveType;

typedef enum {
    FIRST_MOVE_PLAYER,  // ����� ����� ������ (��������)
    FIRST_MOVE_AI       // ��� ����� ������ (������)
} FirstMove;

// ������� �������
typedef struct MoveLog {
    int x, y;           // ����������
    MoveType type;       // ��� ����
    struct MoveLog* next;
} MoveLog;

// ������� (FIFO)
typedef struct {
    MoveLog* head;       // ������ �������
    MoveLog* tail;       // ��������� �������
    int count;           // ������� ���������� (���� 6)
} MoveLogger;

// ������ ��� ���������� ����� �������� ����
typedef struct {
    float zoom;     // �������
    float offsetX;  // �������� �� X
    float offsetY;  // �������� �� Y
} Camera;

// ��������� ����
typedef struct {
    double lastX;       // ��������� ������� X
    double lastY;       // ��������� ������� Y
    int isDragging:2;     // ���� ��������������
} MouseState;

// ������ �������� ����
typedef struct {
    int x, y;          // ����������
    short int symbol:3;   // ������ (0 - �����, 1 - �������, 2 - �����)
} Cell;

// ������� ����
typedef struct {
    Cell* cells;    // ������ ������
    int size;       // ������� ������
    int capacity;   // ���������� ������
} Grid;

// ��������� ����
typedef struct {
    GameDifficulty difficulty;  // ������� ���������(4)
    int fieldSize;             // ������ ���� (0 - �����������)
    int winLineLength;         // ����� ���������� ����� (������� 3)
    FirstMove firstMove;       // ��� ����� ������
} GameSettings;

// �������� ��������� ����������
typedef struct {
    Camera camera;              // ������
    MouseState mouse;           // ��������� ����
    Grid grid;                  // ������� ����
    int selectedCellX;          // ��������� ������ X
    int selectedCellY;          // ��������� ������ Y
    AppMenuState currentState;  // ������� ��������� ����
    int menuSelectedItem:3;       // ��������� ����� �������� ����
    int showHelp:2;               // �������� �������
    int settingsSelectedItem:3;   // ��������� ����� ��������

    stbtt_bakedchar cdata[96];  // ������ ������
    GLuint fontTexture;         // �������� ������
    float fontSize;             // ������ ������
    float saveNotificationTimer;// ������ ����������� � ����������

    GameSettings settings;       // ������� ���������
    GameSettings defaultSettings; // ��������� �� settings.txt


    union {
        int rawResult;                  // ������ ��� � ����� (��� �������������)
        GameResultType resultType;       // �������������� ������
        struct {
            unsigned int isWin : 1;        // ���� ������
            unsigned int isLose : 1;       // ���� ���������
            unsigned int isDraw : 1;       // ���� �����
        };
    } gameResult;
    int winLineStartX;       // ������ ����� (���������� ������)
    int winLineStartY;
    int winLineEndX;         // ����� ����� (���������� ������)
    int winLineEndY;

    MoveLogger logger;  // ������ �����
    int showMoveLog;    // ���� ��� ����������� ���� (1 - ��������, 0 - ������)
} AppState;


// ��������� ������� ��� �����
void makeAIMoveEasy(AppState* state);
void makeAIMoveMedium(AppState* state);
void makeAIMoveHard(AppState* state);
void makeAIMoveExpert(AppState* state);





                                            /*                                     �������                                                */




// �������� ��� ����������
void saveSettings(const GameSettings* settings);

// �������� �����
int checkWinCondition(AppState* state, int symbol, int winLength);
int checkForDraw(AppState* state);

// ������������� ��������� ����������
void initAppState(AppState* state) {
    // ��������� ������
    state->camera.zoom = 1.0f;
    state->camera.offsetX = 0.0f;
    state->camera.offsetY = 0.0f;

    // ��������� ����
    state->mouse.lastX = 0.0;
    state->mouse.lastY = 0.0;
    state->mouse.isDragging = 0;

    // ������� ����
    state->grid.cells = NULL;
    state->grid.size = 0;
    state->grid.capacity = 0;

    // ��������� ������
    state->selectedCellX = 0;
    state->selectedCellY = 0;

    // ��������� ����������
    state->currentState = MENU_MAIN;
    state->showHelp = 0;
    state->menuSelectedItem = 0;
    state->settingsSelectedItem = 0;
    state->fontTexture = 0;
    state->fontSize = 32.0f;
    state->saveNotificationTimer = 0.0f;

    // ������ �����
    state->logger.head = NULL;
    state->logger.tail = NULL;
    state->logger.count = 0;
    state->showMoveLog = 0;  // �� ��������� ���� ������

    
    
    // ��������� �� ���������
    state->settings.difficulty = DIFFICULTY_EASY;
    state->settings.fieldSize = 0;      // ����������� ����
    state->settings.winLineLength = 3;  // ���������� ����� �� 3 ��������
    state->settings.firstMove = FIRST_MOVE_PLAYER; // �� ��������� ����� ����� ������
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
        // ���� ����� ���, ������� ��� � ���������� �����������
        saveSettings(&state->settings);
    }

    // ��������� ������� ��������� ������� ���������
    state->defaultSettings = state->settings;

    state->gameResult.rawResult = 0;          // 0 - ��� ����������, 1 - ������, -1 - ���������
    state->winLineStartX = 0;       // ������ ����� (���������� ������)
    state->winLineStartY = 0;
    state->winLineEndX = 0;         // ����� ����� (���������� ������)
    state->winLineEndY = 0;
}



// ���������� �������� � ����
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

// �������� ����
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
        MoveLog* next = current->next;  // �������� next �� free
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
        // ������� ����� ������ ��� (FIFO)
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


// ������� ���������� ������
void addCell(Grid* grid, int x, int y) {
    //  �������� �������� ���������
    if (grid == NULL) return;

    //  �������� �� ���������
    for (int i = 0; i < grid->size; ++i) {
        if (grid->cells[i].x == x && grid->cells[i].y == y) {
            return;
        }
    }

    //  ���������� ������� ��� �������������
    if (grid->size >= grid->capacity) {
        const int newCapacity = (grid->capacity == 0) ? 4 : grid->capacity * 2;

        // ������� ��������� ��������� ��� �����������
        Cell* const newCells = (Cell*)realloc(grid->cells, newCapacity * sizeof(Cell));
        if (!newCells) {
            fprintf(stderr, "Memory allocation failed\n");
            cleanupGrid(grid);
            exit(1);
        }

        grid->cells = newCells;
        grid->capacity = newCapacity;
    }

    //  ������� ��� VS2019 - ������ ������ ����� ���������
    Cell* target = grid->cells + grid->size;
    target->x = x;
    target->y = y;
    target->symbol = 0;

    //  ����������� ������ ������ ����� �������� ������
    grid->size++;
}


// ������������� � �������� ������
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

    // �������� ��������
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


// ������� ���������� ����
void saveGame(const Grid* grid, const GameSettings* settings) {
    FILE* file = fopen("saves.txt", "w");
    if (!file) {
        fprintf(stderr, "Incorrect file for saving\n");
        return;
    }

    // ��������� ��������� ������ �������, ��������� firstMove
    fprintf(file, "SETTINGS %d %d %d %d\n",
        settings->difficulty,
        settings->fieldSize,
        settings->winLineLength,
        settings->firstMove);  // ��������� ���������� ������� ����

    // ����� ��������� ������
    for (int i = 0; i < grid->size; i++) {
        fprintf(file, "%d %d %d\n", grid->cells[i].x, grid->cells[i].y, grid->cells[i].symbol);
    }

    fclose(file);
}


// �������� ����
int loadGame(Grid* grid, GameSettings* settings) {
    FILE* file = fopen("saves.txt", "r");
    if (!file) {
        file = fopen("saves.txt", "w");
        if (file) fclose(file);
        return 0;
    }

    cleanupGrid(grid);

    // ��������� �� ������ ������ �����
    char line[256];
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SETTINGS", 8) == 0) {
            int temp1;
            int temp2;
            int check = sscanf(line + 8, "%d %d %d %d",
                &temp1,
                &settings->fieldSize,
                &settings->winLineLength,
                &temp2);  // ��������� ������� ����
            settings->difficulty = (GameDifficulty)temp1;
            settings->firstMove = (FirstMove)temp2;

            // ���� �� ������� ��������� firstMove (������ ������ �����)
            if (check < 4) {
                settings->firstMove = FIRST_MOVE_PLAYER; // ������������� �� ���������
            }
        }
        else {
            // ���� ���� ������� �������, ��������� � ������
            fseek(file, 0, SEEK_SET);
            settings->firstMove = FIRST_MOVE_PLAYER; // ������������� �� ���������
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


// ������ ������
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

// ��������� ����������� ����������
void drawSaveNotification(AppState* state, int width, int height) {
    if (state->saveNotificationTimer <= 0.0f) return;

    // ��������� ������� ������� ��������
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ������� � ������� �����������
    float notifWidth = 300;
    float notifHeight = 60;
    float notifX = (width - notifWidth) / 2;
    float notifY = height - 100.0f;

    // ������ �������������� ���
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.5f, 0.1f, 0.9f * (state->saveNotificationTimer / 1.0f)); // ������� ������������
    glBegin(GL_QUADS);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();
    glDisable(GL_BLEND);

    // �����
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

    // ����� �����������
    float textAlpha = state->saveNotificationTimer / 1.0f;
    renderText(state, "Game saved successfully!",
        notifX + 20, notifY + 35,
        0.7f, 1.0f, 1.0f, 1.0f, textAlpha); 

    // ��������������� �������
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


// ��������� �������� ����
void drawMainMenu(AppState* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // ��� ����
    glColor3f(0.1f, 0.1f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // ���������
    float titleWidth = 0;
    const char* title = "Krestiki-Noliki";
    const char* p = title;
    while (*p) {
        
            titleWidth += state->cdata[*p - 32].xadvance;
        
        p++;
    }
    renderText(state, title, (1240 - titleWidth * 1.5f) / 2, 1000, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ������ ����
    const char* items[] = { "New Game", "Load Game", "Settings", "About" };
    for (int i = 0; i < 4; i++) {
        float x = 500;
        float y = 800.0f - i * 200.0f;
        float width = 240;
        float height = 80;

        // ����� ������
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();

        // ������� ������
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

        // ����� ������
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

// ��������� ���� �� �������
void drawAboutScreen(AppState* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // ���
    glColor3f(0.1f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // ���������
    float titleWidth = 0;
    const char* title = "About";
    const char* p = title;
    while (*p) {
        
            titleWidth += state->cdata[*p - 32].xadvance;
        
        p++;
    }
    renderText(state, title, (1240 - titleWidth * 1.5f) / 2, 1000, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // �����
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


//  ������� ��������� ������ ��������
void drawSettingsScreen(AppState* state, GLFWwindow* window) {
    // �������� ���������� ����
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseY = height - mouseY; // ����������� Y
    // ������������ � ����������� ����������� 1240x1240
    mouseX = (mouseX / width) * 1240.0f;
    mouseY = (mouseY / height) * 1240.0f;

    // ��������� ������� ��������
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // ���
    glColor3f(0.2f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // ���������
    renderText(state, "Settings", 500, 1100, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ���������� � ������� ���������
    float yPos = 800;
    float buttonWidth = 120;
    float buttonHeight = 50;
    float arrowSize = 20;
    float valueBoxX = 700;

    //  ��������� ���������
    renderText(state, "Difficulty Level:", 300.0f, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    const char* difficultyOptions[] = { "Easy", "Medium", "Hard", "Expert" };
    for (int i = 0; i < 4; i++) {
        float xPos = 600.0f + i * 130.0f;

        // �������� ��������� ����
        int isHovered = (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

        // ���� ������
        if ((int)state->settings.difficulty == i) {
            glColor3f(0.4f, 0.4f, 0.8f); // ��������� �������
        }
        else if (isHovered) {
            glColor3f(0.3f, 0.3f, 0.6f); // ���������
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f); // �������
        }

        // ������ ������
        glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // �����
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // �����
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

    //  ������ ����
    yPos -= 120;
    renderText(state, "Field Size (0=infinite):", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    char fieldSizeText[32];
    snprintf(fieldSizeText, sizeof(fieldSizeText), "%d", state->settings.fieldSize);

    // �������� ��������� �� ������
    int decHovered = (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);
    int incHovered = (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

    // ������ ����������
    glColor3f(decHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX - 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // ���� ��������
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // ������ ����������
    glColor3f(incHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX + buttonWidth + 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // �����
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // ����� ��������
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

    //  ����� ���������� �����
    yPos -= 120;
    renderText(state, "Win Line Length:", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    char winLineText[32];
    snprintf(winLineText, sizeof(winLineText), "%d", state->settings.winLineLength);

    // �������� ��������� �� ������
    decHovered = (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);
    incHovered = (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

    // ������ ����������
    glColor3f(decHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX - 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // ���� ��������
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // ������ ����������
    glColor3f(incHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX + buttonWidth + 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // �����
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // ����� ��������
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

        // �������� ��������� ����
        int isHovered = (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

        // ���� ������
        if ((int)state->settings.firstMove == i) {
            glColor3f(0.4f, 0.4f, 0.8f); // ��������� �������
        }
        else if (isHovered) {
            glColor3f(0.3f, 0.3f, 0.6f); // ���������
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f); // �������
        }

        // ������ ������
        glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // �����
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // �����
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

    //  ������ ����������
    yPos -= 150;
    float saveX = (1240 - 300) / 2;
    int saveHovered = (mouseX >= saveX && mouseX <= (double)saveX + 300 &&
        mouseY >= yPos && mouseY <= (double)yPos + 60);

    // ��� ������
    glColor3f(saveHovered ? 0.4f : 0.3f, saveHovered ? 0.8f : 0.6f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(saveX, yPos);
    glVertex2f(saveX + 300, yPos);
    glVertex2f(saveX + 300, yPos + 60);
    glVertex2f(saveX, yPos + 60);
    glEnd();

    // �����
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(saveX, yPos);
    glVertex2f(saveX + 300, yPos);
    glVertex2f(saveX + 300, yPos + 60);
    glVertex2f(saveX, yPos + 60);
    glEnd();

    // �����
    renderText(state, "Save Settings", saveX + 70, yPos + 30, 0.9f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ������ �����
    renderText(state, "Back (ESC)", 50, 50, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
}


// ������ ������� ��������� ������ � ����������
void handleSettingsClick(AppState* state, GLFWwindow* window, int button) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    // �������� ���������� ����
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseY = height - mouseY; // ����������� Y
    // ������������ � ����������� ����������� 1240x1240
    mouseX = (mouseX / width) * 1240.0f;
    mouseY = (mouseY / height) * 1240.0f;

    // ���������� ���������
    float yPos = 800;
    float buttonWidth = 120;
    float buttonHeight = 50;
    float valueBoxX = 700;

    //  �������� ������ �� ������ ���������
    for (int i = 0; i < 4; i++) {
        float xPos = 600.0f + i * 130.0f;
        if (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
            state->settings.difficulty = i;
            return;
        }
    }

    //  �������� ������ �� ������� ����
    yPos -= 120;

    // ������ ����������
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

    // ������ ����������
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

    //  �������� ������ �� ����� �����
    yPos -= 120;

    // ������ ����������
    if (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
        state->settings.winLineLength = (state->settings.winLineLength > 3) ? state->settings.winLineLength - 1 : 3;
        return;
    }

    // ������ ����������
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
    

    //  �������� ����� �� ������ ����������
    yPos -= 150;
    float saveX = (1240 - 300) / 2;
    if (mouseX >= saveX && mouseX <= (double)saveX + 300 &&
        mouseY >= yPos && mouseY <= (double)yPos + 60) {
        saveSettings(&state->settings);
        state->defaultSettings = state->settings; // ��������� ��������� ���������
        return;
    }

    //  �������� ����� �� ������ ����� (������� ����� ����)
    if (mouseX < 200 && mouseY < 100) {
        state->currentState = MENU_MAIN;
    }
}


// ������ ���������� ������� _itoa �.�. ��� ������ ����
static void int_to_str(int value, char* str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char* ptr = str;
    int is_negative = 0;

    // ��������� ������������� ����� ��� base 10
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    // ������������ 0 ����, ����� ����� ������ ������
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    // ���������� ����� � �������� �������
    char* start = ptr;
    while (value != 0) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
        value /= base;
    }

    // ��������� ���� ����� ��� ������������� �����
    if (is_negative) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    // ������������� ������
    ptr--;
    while (start < ptr) {
        char tmp = *start;
        *start++ = *ptr;
        *ptr-- = tmp;
    }
}


// ��������� ���� ������
void drawHelpWindow(AppState* state) {
    // ��������� ������� ������� ��������
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ������ �������������� ���
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

    // �����
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(300, 170);
    glVertex2f(940, 170);
    glVertex2f(940, 900);
    glVertex2f(300, 900);
    glEnd();

    // ���������
    renderText(state, "Game Controls", 450, 850, 1.2f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ����� ����������
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

    // �������� ��������� ������������� ��������
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

    // ������������ ���������
    renderText(state, instructions[6], 320, 450, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ���������
    float x = 320;
    float y = 400;
    renderText(state, instructions[7], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, difficultyNames[state->settings.difficulty], x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ������ ����
    y -= 50;
    renderText(state, instructions[8], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, fieldSizeText, x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ����� �����
    y -= 50;
    renderText(state, instructions[9], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, winLineText, x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ��������� �����
    renderText(state, instructions[11], 320, 200, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ��������������� �������
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void drawMoveLog(AppState* state) {
    // 1. ��������� ������ � ������������
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1); // ������ ���� 1240x1240
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 2. ��������� ����
    float x = 50.0f, y = 100.0f;
    float width = 300.0f, height = 200.0f;

    // 3. ��� ���� (��������������)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.1f, 0.2f, 0.9f); // �����-����� � �������������
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 4. ����� ����
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f); // �����
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 5. ����� (���������)
    renderText(state, "Last Moves:", x + 10.0f, y + height - 30.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // 6. ������ �����
    MoveLog* current = state->logger.head;
    float textY = y + height - 60.0f; // ��������� ������� ��� ������
    int count = 0;

    while (current != NULL && count < 6) {
        char buf[50];
        const char* type = (current->type == MOVE_PLAYER) ? "Player" : "AI";
        snprintf(buf, sizeof(buf), "%s: (%d, %d)", type, current->x, current->y);

        // ���������, �� ����� �� �� ������� ����
        if (textY < y + 10.0f) break;

        renderText(state, buf, x + 10.0f, textY, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
        textY -= 25.0f; // ������� ����
        current = current->next;
        count++;
    }

    // 7. ��������������� �������
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_BLEND);
}


// ��������� � ������ ������� ���� � ������
void drawHelpHint(AppState* state, int width, int height) {
    // ��������� ������� ������� ��������
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ������� � ������� ����� ���������
    float hintWidth = 200;
    float hintHeight = 50;
    float hintX = width - hintWidth - 20;
    float hintY = height - hintHeight - 20;

    // ������ �������������� ���
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

    // �����
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(hintX, hintY);
    glVertex2f(hintX + hintWidth, hintY);
    glVertex2f(hintX + hintWidth, hintY + hintHeight);
    glVertex2f(hintX, hintY + hintHeight);
    glEnd();

    // ����� ���������
    renderText(state, "Press 'H' for Help", hintX + 10, hintY + 25, 0.6f, 1.0f, 1.0f, 1.0f, 1.0f);

    // ��������������� �������
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ��������� ���������� ����
void drawGrid(float visibleLeft, float visibleRight,
    float visibleBottom, float visibleTop, float zoom, int fieldSize) {
    // ������ ������ � ������� �����������
    float cellSize = 2.0f / (10.0f * zoom);

    // ���������� ������� ������� ������� � �������
    int startX = (int)(visibleLeft / cellSize) - 1;
    int endX = (int)(visibleRight / cellSize) + 1;
    int startY = (int)(visibleBottom / cellSize) - 1;
    int endY = (int)(visibleTop / cellSize) + 1;

    // ���� ���� ����������, ������������ ������� ���������
    if (fieldSize > 0) {
        startX = (startX < -fieldSize / 2) ? -fieldSize / 2 : startX;
        endX = (endX > fieldSize / 2) ? fieldSize / 2 : endX;
        startY = (startY < -fieldSize / 2) ? -fieldSize / 2 : startY;
        endY = (endY > fieldSize / 2) ? fieldSize / 2 : endY;
    }

    // ������������ ������ ������� ������
    for (int x = startX; x <= endX; ++x) {
        for (int y = startY; y <= endY; ++y) {
            // ���� ���� ����������, ���������� ������ �� ���������
            if (fieldSize > 0 && (abs(x) > fieldSize / 2 || abs(y) > fieldSize / 2)) {
                continue;
            }

            // ��������� ���������� ����� ������
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

// ��������� ��������� ��������� ������
void drawSelectedCell(int x, int y, float cellSize) {
    // ��������� ���������� ����� ��������� ������
    float x1 = x * cellSize;
    float y1 = y * cellSize;
    float x2 = x1 + cellSize;
    float y2 = y1 + cellSize;
    glColor3f(1.0, 1.0, 0.0);

    // ������������ ���������
    glLineWidth(5.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// ��������� �������� � ������
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

// ��������� ������ � ������
void drawCircle(float x1, float y1, float x2, float y2) {
    glColor3f(0.0, 0.0, 1.0);  // ����� ���� ��� ������

    float centerX = (x1 + x2) / 2.0f;
    float centerY = (y1 + y2) / 2.0f;
    float outerRadius = (x2 - x1) / 2.5f;  // ������� ������
    float innerRadius = outerRadius * 0.7f; // ���������� ������ ��� ������� �����

    // ������ ����� � ������� �������������
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 360; i += 5) {  // ��� 5 �������� ����� ��������� ��� ������� ���������
        float angle = i * 3.14159f / 180.0f;

        // ������� �����
        glVertex2f((GLfloat)centerX + (GLfloat)outerRadius * (GLfloat)cos(angle),
            (GLfloat)centerY + (GLfloat)outerRadius * (GLfloat)sin(angle));

        // ���������� �����
        glVertex2f((GLfloat)centerX + (GLfloat)innerRadius * (GLfloat)cos(angle),
            (GLfloat)centerY + (GLfloat)innerRadius * (GLfloat)sin(angle));
    }
    glEnd();
}

// ��������� ���� ������� ����
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

    // ����������� ����
    if (state->camera.zoom < 0.4f) state->camera.zoom = 0.4f;
    if (state->camera.zoom > 3.0f) state->camera.zoom = 3.0f;
}


// ��������� ������� ������ ����
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS && state->currentState == MENU_SETTINGS) {
        handleSettingsClick(state, window, button);
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (state->currentState == MENU_MAIN) {
            // ��������� ������ � ��������� � ����
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (mods == 1) {
                int kostil = 0;
                kostil++;
            }
            // ����������� ���������� ���� � ���������� ������ (1240x1240)
            ypos = height - ypos; // ����������� Y
            float menuX = ((float)xpos / (float)width) * 1240.0f;
            float menuY = ((float)ypos / (float)height) * 1240.0f;

            // ���������, ����� ������ ��� ��������
            if (menuX >= 500 && menuX <= 740) { // ������ ������ 240
                if (menuY >= 800 && menuY <= 880) { // New Game (������ ������)
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
                        
                        // ���� ��� ����� ������, ������ ��� ��� �����
                        if (state->settings.firstMove == FIRST_MOVE_AI) {
                            // ����������� ������ ��� ������� ���� ����
                                addCell(&state->grid, 0, 0);
                                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                                logMove(&state->logger, 0, 0, MOVE_AI);
                        }

                    }
                }
                else if (menuY >= 600 && menuY <= 680) { // Load Game (������ ������)
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
                else if (menuY >= 400 && menuY <= 480) { // Settings (������ ������)
                    state->menuSelectedItem = 2;
                    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                        state->currentState = MENU_SETTINGS;
                    }
                }
                else if (menuY >= 200 && menuY <= 280) { // About (��������� ������)
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

// ��������� ����������� �������
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
        // ��������� ��������� � ������� ����
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ypos = height - ypos; // ����������� Y
        float menuX = ((float)xpos / (float)width) * 1240.0f;
        float menuY = ((float)ypos / (float)height) * 1240.0f;

        // ���������, ����� ������ ��� ��������
        if (menuX >= 500 && menuX <= 740) { // ������ ������ 240
            if (menuY >= 800 && menuY <= 880) { // New Game (������ ������)
                
                state->menuSelectedItem = 0;

            }
            else if (menuY >= 600 && menuY <= 680) { // Load Game (������ ������)
                state->menuSelectedItem = 1;
            }
            else if (menuY >= 400 && menuY <= 480) { // Settings (������ ������)
                state->menuSelectedItem = 2;
            }
            else if (menuY >= 200 && menuY <= 280) { // About (��������� ������)
                state->menuSelectedItem = 3;
            }
        }
    }
}
// �������� �����
int checkForDraw(AppState* state) {
    if (state->settings.fieldSize <= 0) return 0; // ����������� ���� - ����� ����������

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
                return 0; // ����� ������ ������ - ����� ���
            }
        }
    }
    return 1; // ��� ������ ��������� - �����
}

// ��������� ������� ������
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
            state->menuSelectedItem = (state->menuSelectedItem - 1 + 4) % 4; // �� ������� � ������� ������� �������� menuSelectedItem �� enum
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
                // ���������� ��������� �� defaultSettings
                state->settings = state->defaultSettings;
                state->currentState = MENU_GAME;
                state->gameResult.rawResult = 0;

                // ���� ��� ����� ������, ������ ��� ��� �����
                if (state->settings.firstMove == FIRST_MOVE_AI) {
                    // ����������� ������ ��� ������� ���� ����
                    if (state->settings.fieldSize > 0) {
                        // ��� ������������� ���� - ������ � �����
                        addCell(&state->grid, 0, 0);
                        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                        logMove(&state->logger, 0, 0, MOVE_AI);
                    }
                    else {
                        // ��� ������������ ���� - ������ � (0,0)
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
                    state->gameResult.rawResult = 0; // ���������� ��������� ����
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
                state->gameResult.rawResult = 0; // ���������� ��������� ���� ��� ������
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
            if (state->gameResult.rawResult != 0) break; // ���� ��� ���������

            // ���������� ������ ������ � ���� � ����������� �� ����, ��� ����� ������
            int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
            aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;

            // ���������, �� ������ �� ������
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

            // ������ ��� ������
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

            // �������� ������ ����� ����
            if (checkWinCondition(state, playerSymbol, state->settings.winLineLength)) {
                state->gameResult.isWin = 1;
                break;
            }

            // �������� �����
            if (checkForDraw(state)) {
                state->gameResult.isDraw = 1;
                break;
            }

            // ��� ���� (������ ���� ���� �� ���������)
            if (state->gameResult.rawResult == 0) {
                // ���������� ������� ��� ���� ���� � ����������� �� ���������
                void (*aiMoveFunc)(AppState*) = NULL;
                switch (state->settings.difficulty) {
                case DIFFICULTY_EASY: aiMoveFunc = makeAIMoveEasy; break;
                case DIFFICULTY_MEDIUM: aiMoveFunc = makeAIMoveMedium; break;
                case DIFFICULTY_HARD: aiMoveFunc = makeAIMoveHard; break;
                case DIFFICULTY_EXPERT: aiMoveFunc = makeAIMoveExpert; break;
                }

                if (aiMoveFunc) {
                    aiMoveFunc(state);

                    // �������� ������ ���� ����� ��� ����
                    if (checkWinCondition(state, aiSymbol, state->settings.winLineLength)) {
                        state->gameResult.isLose = 1;
                    }
                    // �������� �����
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
                state->saveNotificationTimer = 1.0; // ������������� ������ �� 2 �������
                break;
            }
            break;
        case GLFW_KEY_ESCAPE:
            if (state->currentState == MENU_GAME) {
                // ��������������� ��������� �� defaultSettings
                cleanupMoveLogger(&state->logger);
                state->settings = state->defaultSettings;
                state->currentState = MENU_MAIN;
                state->gameResult.rawResult = 0;
            }
            break;
        case GLFW_KEY_H:
            state->showHelp = !state->showHelp; // ����������� ����������� ������
            break;
        case GLFW_KEY_M:  // M - ��������/������ ���
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

// ��������� �������� ���������
void drawWinLine(AppState* state, int width, int height) {
    if (state->gameResult.rawResult == 0) return;

    // ��������� �������
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ������� � ������� �����������
    float notifWidth = 400;
    float notifHeight = 100;
    float notifX = (width - notifWidth) / 2;
    float notifY = height - 150.0f;

    // ������ �������������� ���
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (state->gameResult.isWin == 1) { // ������
        glColor4f(0.1f, 0.5f, 0.1f, 0.9f);
    }
    else if (state->gameResult.isLose == 1) { // ���������
        glColor4f(0.5f, 0.1f, 0.1f, 0.9f);
    }
    else { // �����
        glColor4f(0.3f, 0.3f, 0.3f, 0.9f);
    }

    glBegin(GL_QUADS);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();

    // �����
    glColor4f(0.8f, 0.8f, 0.0f, 0.9f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();

    // ����� �����������
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

    // ��������������� �������
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ��������� �������� �����
void drawWinningLine(AppState* state) {

    // ����������, ��� �������
    int winnerSymbol = 0;
    if (state->gameResult.isWin) {
        winnerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    }
    else if (state->gameResult.isLose) {
        winnerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    }

    // ������������� ���� � ����������� �� ����, ��� �������
    if (winnerSymbol == 1) { // �������
        int isPlayer = (state->settings.firstMove == FIRST_MOVE_PLAYER);
        if (isPlayer) {
            glColor3f(1.0f, 0.0f, 0.0f); // ������� ��� ������
        }
        else {
            glColor3f(0.5f, 0.0f, 0.5f); // ���������� ��� ����
        }
    }
    else { // �����
        int isPlayer = (state->settings.firstMove != FIRST_MOVE_PLAYER);
        if (isPlayer) {
            glColor3f(0.0f, 0.0f, 1.0f); // ����� ��� ������
        }
        else {
            glColor3f(0.0f, 1.0f, 1.0f); // ������� ��� ����
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ������ ����� ����� �������� ������
    glColor3f(1.0f, 1.0f, 0.0f); // ������ ����
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







                                                    /*                         ���������                                  */


// ��������, ���� �� ���������� ����� �� symbol �������� �����
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

            // ��������� � ����� �����������
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

            // ��������� � ��������������� �����������
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



// ������� ��� ���� ���� (������ �������), ������� ������ � ������� ����� �������� �����
void makeAIMoveEasy(AppState* state) {
    
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }


    // �������� ��� ������
    int crossCount = 0;
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) crossCount++;
    }

    

    // �������� ���������
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

    // �������� ��������� ����� � ��������� ���������
    int attempts = 0;
    const int maxAttempts = state->settings.winLineLength;

    while (attempts < maxAttempts) {
        // �������� ��������� ����������� � ���������� (�� ������ winLineLength)
        int dx = (rand() % (2 * state->settings.winLineLength + 1)) - state->settings.winLineLength;
        int dy = (rand() % (2 * state->settings.winLineLength + 1)) - state->settings.winLineLength;

        // ��������, ��� �� �� �������� �� �����
        if (dx == 0 && dy == 0) continue;

        int newX = targetX + dx;
        int newY = targetY + dy;

        // ���������, ��� ������ �������� � � �������� ���� (���� ���� ����������)
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
                // ���������, ��� � �������� ������������� ����
                if (abs(newX) <= state->settings.fieldSize / 2 &&
                    abs(newY) <= state->settings.fieldSize / 2) {
                    addCell(&state->grid, newX, newY);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                    logMove(&state->logger, newX, newY, MOVE_AI);
                    return;
                }
            }
            else {
                // ��� ������������ ���� ������ ���������
                addCell(&state->grid, newX, newY);
                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                logMove(&state->logger, newX, newY, MOVE_AI);
                return;
            }
        }

        attempts++;
    }

    // ���� �� ������� ����� ����� �����, ������ � ��������� ��������� ������
    if (state->settings.fieldSize > 0) {
        // ��� ������������� ����
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

// ��� ������� ��������� ������ �������, ����� ��������� ������
void makeAIMoveMedium(AppState* state) {
    

    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. ���������, ���� �� ���������� ��� ��� ���� (������)
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // ���� ���� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // ��������� ��� �����������
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �����
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
                                    found = -1; // ������� ������
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // ������ ������
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // ������� �� ����
                        }
                    }

                    // ���� ����� ���������� ��� (����� ���� ������ ������ � ����� ������ �����)
                    if (count == state->settings.winLineLength - 1 && emptyCount == 1) {
                        // ���������, ��� ������ � �������� ����
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

    // 2. ���������, ����� �� ����������� ���������� ��� ������
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // ���� �������� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �������
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
                                    found = -1; // ����� ������
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // ������ ������
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // ����� �� ����
                        }
                    }

                    // ���� ����� ����������� (����� ���� ������ ������ � ����� ������ �����)
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

    // 3. ������ �����, ���� �� ��������
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

    // 4. ��������� ����� ����� ������ � ������� (���������� ������)
    int strategy = rand() % 2; // 0 - �����, 1 - ������

    if (strategy == 0) { // ����� - ��������� ����� �� ����� �������
        // �������� ��� ���� ������
        int nolikCount = 0;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].symbol == aiSymbol) nolikCount++;
        }

        if (nolikCount > 0) {
            // �������� ��������� �����
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

            // �������� ��������� ����� (� ������� 1 ������)
            for (int attempt = 0; attempt < 8; attempt++) {
                int dx = (rand() % 3) - 1; // -1, 0 ��� 1
                int dy = (rand() % 3) - 1;

                if (dx == 0 && dy == 0) continue;

                int newX = targetX + dx;
                int newY = targetY + dy;

                // ���������, ��� ������ ��������
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == newX &&
                        state->grid.cells[i].y == newY) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // ��������� ������� ����
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
    else { // ������ - ��������� ����� � ���������
        // �������� ��� ��������
        int krestikCount = 0;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].symbol == playerSymbol) krestikCount++;
        }

        if (krestikCount > 0) {
            // �������� ��������� �������
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

            // �������� ��������� ����� (� ������� 1 ������)
            for (int attempt = 0; attempt < 8; attempt++) {
                int dx = (rand() % 3) - 1;
                int dy = (rand() % 3) - 1;

                if (dx == 0 && dy == 0) continue;

                int newX = targetX + dx;
                int newY = targetY + dy;

                // ���������, ��� ������ ��������
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == newX &&
                        state->grid.cells[i].y == newY) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // ��������� ������� ����
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

    // ���� ��� ��������� �� ���������, ������ ��������� ���
    makeAIMoveEasy(state);
}


// ��� �������� ������, �������� �� ����������, �������� ����������, ��������� � ������
void makeAIMoveHard(AppState* state) {
    

    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. ���������, ���� �� ���������� ��� ��� ���� (������)
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // ���� ���� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // ��������� ��� �����������
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �����
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
                                    found = -1; // ������� ������
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // ������ ������
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // ������� �� ����
                        }
                    }

                    // ���� ����� ���������� ��� (����� ���� ������ ������ � ����� ������ �����)
                    if (count >= state->settings.winLineLength - 1 && emptyCount == 1) {
                        // ���������, ��� ������ � �������� ����
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

    // 2. ���������, ����� �� ����������� ���������� ��� ������
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // ���� �������� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �������
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
                                    found = -1; // ����� ������
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // ������ ������
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // ����� �� ����
                        }
                    }

                    // ���� ����� ����������� (����� �� winLength-1 ��������� � ����� ������)
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
    // 4. ������ �����, ���� �� �������� (������ ��� ������������� ����)
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

    // 3. ����� ������������� ������ ������� (������� �����)
    // ������� ���� ���� �����, ������� ����� ����������
    int bestScore = -1;
    int bestX = 0, bestY = 0;

    // ��������� ��� ��������� ������ ������
    for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
        for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
            // ��� ������������� ���� ���������� ������ �� ���������
            if (state->settings.fieldSize > 0 &&
                (abs(x) > state->settings.fieldSize / 2 ||
                    abs(y) > state->settings.fieldSize / 2)) {
                continue;
            }

            // ���������, ��� ������ ��������
            int cellFree = 1;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                    cellFree = 0;
                    break;
                }
            }

            if (!cellFree) continue;

            // ��������� ������ �� ���� ������������
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };
            int cellScore = 0;

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                int myCount = 0;   // ������
                int oppCount = 0;   // ��������
                int emptyCount = 0;  // ������

                for (int step = -state->settings.winLineLength + 1;
                    step < state->settings.winLineLength; step++) {
                    if (step == 0) continue; // ���������� ����������� ������ (��� ������)

                    int currentX = x + dx * step;
                    int currentY = y + dy * step;

                    // ��� ������������� ���� ��������� �������
                    if (state->settings.fieldSize > 0 &&
                        (abs(currentX) > state->settings.fieldSize / 2 ||
                            abs(currentY) > state->settings.fieldSize / 2)) {
                        continue;
                    }

                    // ��������� ���������� ������
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

                // ��������� ��������� �����
                if (myCount > 0 && oppCount == 0) {
                    // ����� � ������ ��������� - ������� �����������
                    cellScore += myCount * myCount;
                }
                else if (oppCount > 0 && myCount == 0) {
                    // ����� � ��������� ���������� - ����� �����������
                    cellScore += oppCount * oppCount;
                }
            }

            // ��������� ������ ������
            if (cellScore > bestScore) {
                bestScore = cellScore;
                bestX = x;
                bestY = y;
            }
        }
    }

    // ���� ����� ������� �������, ������ ���
    if (bestScore > 0) {
        addCell(&state->grid, bestX, bestY);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, bestX, bestY, MOVE_AI);
        return;
    }

    

    // 5. ���� ������ ��������������� �� �������, ������ ��� ����� � ������������� ��������
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // ����� �����
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // ��������� ��� �������� ������
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;

                    int newX = x + dx;
                    int newY = y + dy;

                    // ��������� ������� ��� ������������� ����
                    if (state->settings.fieldSize > 0 &&
                        (abs(newX) > state->settings.fieldSize / 2 ||
                            abs(newY) > state->settings.fieldSize / 2)) {
                        continue;
                    }

                    // ���������, ��� ������ ��������
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

    // 6. ���� ��� ��������� �� ���������, ������ ��������� ���
    makeAIMoveMedium(state);
}


// ��������������� ������� ��� ������ ������� (��� ���������)
int evaluatePosition(AppState* state, int symbol, int opponentSymbol) {
    int score = 0;
    

    // ��������� ��� ������ � ����� ��������
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol != symbol) continue;

        int x = state->grid.cells[i].x;
        int y = state->grid.cells[i].y;

        int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

        for (int d = 0; d < 4; d++) {
            int dx = directions[d][0];
            int dy = directions[d][1];

            // ��������� ����� � ����� ������������
            int count = 1; // ��� ���� ���� ��� ������
            int openEnds = 0;
            int blockedEnds = 0;

            // ��������� � ����� �����������
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
                    // ��������� ������� ��� ������������� ����
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

            // ��������� � ��������������� �����������
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
                    // ��������� ������� ��� ������������� ����
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

            // ��������� ������������� �����
            if (count >= state->settings.winLineLength) {
                return (symbol == 2) ? 10000 : -10000; // ������/���������
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

// �������� � �����-���� ����������
int minimax(AppState* state, int depth, int isMaximizing, int alpha, int beta, int* bestX, int* bestY, int aiSymbol, int playerSymbol) {
    // ��������� ������������ ���������
    int playerWin = checkWinCondition(state, playerSymbol, state->settings.winLineLength);
    int aiWin = checkWinCondition(state, aiSymbol, state->settings.winLineLength);
    int draw = checkForDraw(state);

    if (aiWin) return 100000 - depth; // ��� ������, ��� ������ ����
    if (playerWin) return -100000 + depth;
    if (draw) return 0;
    if (depth == 0) return evaluatePosition(state, playerSymbol, aiSymbol);

    if (isMaximizing) {
        int maxEval = -1000000;

        // ���������� ��������� ����
        for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
            for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
                // ��������� ������� ��� ������������� ����
                if (state->settings.fieldSize > 0 &&
                    (abs(x) > state->settings.fieldSize / 2 ||
                        abs(y) > state->settings.fieldSize / 2)) {
                    continue;
                }

                // ���������, ��� ������ ��������
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // ������ ���
                    addCell(&state->grid, x, y);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;

                    int eval = minimax(state, depth - 1, 0, alpha, beta, NULL, NULL, aiSymbol, playerSymbol);

                    // �������� ���
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

        // ���������� ��������� ����
        for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
            for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
                // ��������� ������� ��� ������������� ����
                if (state->settings.fieldSize > 0 &&
                    (abs(x) > state->settings.fieldSize / 2 ||
                        abs(y) > state->settings.fieldSize / 2)) {
                    continue;
                }

                // ���������, ��� ������ ��������
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // ������ ���
                    addCell(&state->grid, x, y);
                    state->grid.cells[state->grid.size - 1].symbol = playerSymbol;

                    int eval = minimax(state, depth - 1, 1, alpha, beta, NULL, NULL, aiSymbol, playerSymbol);

                    // �������� ���
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


// �������� ������� ��� ����������� ������, ���������� �������� ����� ��������� � ����� ��������� � ����� ���� ����������, �������� �����, �� �� ���������� �� ������� ������ � �����
void makeAIMoveExpert(AppState* state) {
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;

    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. ���������, ���� �� ���������� ��� ��� ���� (������)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // ���� ���� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �����
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

                        if (found == 0) { // ������ ������
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;
                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) break;
                    }

                    // ���������� ��� (�� ������� ����� ������)
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

    // 2. ���������, ����� �� ����������� ���������� ��� ������ (�� ������� ����� ������)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // ���� �������� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �������
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

                        if (found == 0) { // ������ ������
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;
                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) break;
                    }

                    // ������� ���������� (�� ������� ����� ������)
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

    // 3. ��������� ������� �������� (�� ������� ���� �����)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // ���� �������� ������
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // ��������� ����� � ����� ������������
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // ��� ���� ���� �������
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

                        if (found == 0) { // ������ ������
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

                    // ������� �������� (�� ������� ���� �����)
                    if (count >= state->settings.winLineLength - 2 && emptyCount >= 2) {
                        // ������������ ����������� ������, ������� ������� ������ ������������ ��� ���
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

    // 4. ������������ �������� ��� �������������� �����
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

    // 5. ���� ��� ��������� �� ���������, ���������� ��������� �� �������� ������
    makeAIMoveHard(state);
}



int main() {
    
    
    
    // ������������� GLFW
    if (!glfwInit()) {
        fprintf(stderr, "������ ������������� GLFW\n");
        return -1;
    }

    // ������� ����
    GLFWwindow* window = glfwCreateWindow(1240, 1240, "Krestiki-Noliki", NULL, NULL);
    if (!window) {
        fprintf(stderr, "������ �������� ����\n");
        glfwTerminate();
        return -1;
    }

    // ������ �������� �������
    glfwMakeContextCurrent(window);

    // ������������� ��������� ����������
    AppState state;
    initAppState(&state);
    

    // ������������� ������
    if (!initText(&state, "text/arial.ttf")) {
        fprintf(stderr, "�� ������� ��������� �����\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowUserPointer(window, &state);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);

    // �������� ���� ����������
    while (!glfwWindowShouldClose(window)) {
        // ������� ������
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // �������� ������� ����
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (state.saveNotificationTimer > 0.0f) {
            state.saveNotificationTimer -= 0.005f; // ��������� �� ����� ����� 
        }
        


        // �������� ��� ������������ � ����������� �� ���������
        switch (state.currentState) {
        case MENU_MAIN:
            drawMainMenu(&state);
            break;

        case MENU_GAME: {
            // ������������� ������� ��������
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            // ������������ ������� �������
            float aspectRatio = (float)width / (float)height;
            float visibleLeft = -1.0f * state.camera.zoom * aspectRatio + state.camera.offsetX;
            float visibleRight = 1.0f * state.camera.zoom * aspectRatio + state.camera.offsetX;
            float visibleBottom = -1.0f * state.camera.zoom + state.camera.offsetY;
            float visibleTop = 1.0f * state.camera.zoom + state.camera.offsetY;

            glOrtho(visibleLeft, visibleRight, visibleBottom, visibleTop, -1.0, 1.0);

            // ������������� ������� ������
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glColor3f(0.0, 0.0, 0.0);

            // ������������ ����
            drawGrid(visibleLeft, visibleRight, visibleBottom, visibleTop, state.camera.zoom, state.settings.fieldSize);

            // ������������ ��������� ������
            float cellSize = 2.0f / (10.0f * state.camera.zoom);
            drawSelectedCell(state.selectedCellX, state.selectedCellY, cellSize);

            // ������������ �������
            for (int i = 0; i < state.grid.size; i++) {
                float x1 = state.grid.cells[i].x * cellSize;
                float y1 = state.grid.cells[i].y * cellSize;
                float x2 = x1 + cellSize;
                float y2 = y1 + cellSize;
                
                if (state.grid.cells[i].symbol == 1) { // ������� 
                    drawCross(x1, y1, x2, y2);
                }
                else if (state.grid.cells[i].symbol == 2) { // �����
                    drawCircle(x1, y1, x2, y2);
                }
            }
            if (state.currentState == MENU_SETTINGS) {
                drawSettingsScreen(&state, window);
        }
            if (state.saveNotificationTimer > 0.0f) { // ������ ����� � �����
                drawSaveNotification(&state, width, height);
            }
            if (state.gameResult.rawResult  != 0) { // �������� �����
                drawWinLine(&state, width, height);
                if (state.gameResult.isDraw != 1) {
                    drawWinningLine(&state);
                }
            }

            // ������ ��������� � ������ (������ ���� ���� ������ �� ������������)
            if (!state.showHelp) {
                drawHelpHint(&state, width, height);
            }

            if (state.showMoveLog) {
                drawMoveLog(&state);
            }

            // ���� ����� �������� ������, ������ ������ �����
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

        // ������ ������ � ������������ �������
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ����������� �������
    glDeleteTextures(1, &state.fontTexture);
    cleanupGrid(&state.grid);
    cleanupMoveLogger(&state.logger);
    glfwTerminate();
    
    return 0;
}