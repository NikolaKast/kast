#define _CRT_SECURE_NO_WARNINGS
#include "include/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define STB_TRUETYPE_IMPLEMENTATION
#include "text/stb_truetype.h"
// Áèáëèîòåêà äëÿ îòðèñîâêè áóêîâîê è ÷èñåëîê, äîïîëíèòåëüíî ïîäãðóæåí øðèôò arial.ttf



                                                    /*                                ÑÒÐÓÊÒÓÐÛ                                     */
// Ñîñòîÿíèÿ ìåíþ ïðèëîæåíèÿ
typedef enum {
    MENU_MAIN,     // Ãëàâíîå ìåíþ
    MENU_GAME,     // Èãðîâîé ýêðàí
    MENU_ABOUT,    // Ýêðàí "Î ïðîãðàììå"
    MENU_SETTINGS  // Ýêðàí íàñòðîåê
} AppMenuState;

// Óðîâíè ñëîæíîñòè èãðû
typedef enum {
    DIFFICULTY_EASY,    // Ëåãêèé
    DIFFICULTY_MEDIUM,  // Ñðåäíèé
    DIFFICULTY_HARD,    // Ñëîæíûé
    DIFFICULTY_EXPERT   // Ýêñïåðò
} GameDifficulty;

typedef enum {
    GAME_RESULT_NONE = 0,   // Èãðà ïðîäîëæàåòñÿ
    GAME_RESULT_WIN = 1,    // Ïîáåäà
    GAME_RESULT_LOSE = -1,  // Ïîðàæåíèå
    GAME_RESULT_DRAW = 2    // Íè÷üÿ
} GameResultType;

// Òèï õîäà (èãðîê/áîò)
typedef enum {
    MOVE_PLAYER,  // Õîä èãðîêà (êðåñòèê)
    MOVE_AI       // Õîä áîòà (íîëèê)
} MoveType;

typedef enum {
    FIRST_MOVE_PLAYER,  // Èãðîê õîäèò ïåðâûì (êðåñòèêè)
    FIRST_MOVE_AI       // Áîò õîäèò ïåðâûì (íîëèêè)
} FirstMove;

// Ýëåìåíò î÷åðåäè
typedef struct MoveLog {
    int x, y;           // Êîîðäèíàòû
    MoveType type;       // Òèï õîäà
    struct MoveLog* next;
} MoveLog;

// Î÷åðåäü (FIFO)
typedef struct {
    MoveLog* head;       // Ïåðâûé ýëåìåíò
    MoveLog* tail;       // Ïîñëåäíèé ýëåìåíò
    int count;           // Òåêóùåå êîëè÷åñòâî (ìàêñ 6)
} MoveLogger;

// Êàìåðà äëÿ óïðàâëåíèÿ âèäîì èãðîâîãî ïîëÿ
typedef struct {
    float zoom;     // Ìàñøòàá
    float offsetX;  // Ñìåùåíèå ïî X
    float offsetY;  // Ñìåùåíèå ïî Y
} Camera;

// Ñîñòîÿíèå ìûøè
typedef struct {
    double lastX;       // Ïîñëåäíÿÿ ïîçèöèÿ X
    double lastY;       // Ïîñëåäíÿÿ ïîçèöèÿ Y
    int isDragging:2;     // Ôëàã ïåðåòàñêèâàíèÿ
} MouseState;

// Êëåòêà èãðîâîãî ïîëÿ
typedef struct {
    int x, y;          // Êîîðäèíàòû
    short int symbol:3;   // Ñèìâîë (0 - ïóñòî, 1 - êðåñòèê, 2 - íîëèê)
} Cell;

// Èãðîâîå ïîëå
typedef struct {
    Cell* cells;    // Ìàññèâ êëåòîê
    int size;       // Òåêóùèé ðàçìåð
    int capacity;   // Âûäåëåííàÿ ïàìÿòü
} Grid;

// Íàñòðîéêè èãðû
typedef struct {
    GameDifficulty difficulty;  // Óðîâåíü ñëîæíîñòè(4)
    int fieldSize;             // Ðàçìåð ïîëÿ (0 - áåñêîíå÷íîå)
    int winLineLength;         // Äëèíà âûèãðûøíîé ëèíèè (ìèíèìóì 3)
    FirstMove firstMove;       // Êòî õîäèò ïåðâûì
} GameSettings;

// Îñíîâíîå ñîñòîÿíèå ïðèëîæåíèÿ
typedef struct {
    Camera camera;              // Êàìåðà
    MouseState mouse;           // Ñîñòîÿíèå ìûøè
    Grid grid;                  // Èãðîâîå ïîëå
    int selectedCellX;          // Âûáðàííàÿ êëåòêà X
    int selectedCellY;          // Âûáðàííàÿ êëåòêà Y
    AppMenuState currentState;  // Òåêóùåå ñîñòîÿíèå ìåíþ
    int menuSelectedItem:3;       // Âûáðàííûé ïóíêò ãëàâíîãî ìåíþ
    int showHelp:2;               // Ïîêàçàòü ñïðàâêó
    int settingsSelectedItem:3;   // Âûáðàííûé ïóíêò íàñòðîåê

    stbtt_bakedchar cdata[96];  // Äàííûå øðèôòà
    GLuint fontTexture;         // Òåêñòóðà øðèôòà
    float fontSize;             // Ðàçìåð øðèôòà
    float saveNotificationTimer;// Òàéìåð óâåäîìëåíèÿ î ñîõðàíåíèè

    GameSettings settings;       // Òåêóùèå íàñòðîéêè
    GameSettings defaultSettings; // Íàñòðîéêè èç settings.txt


    union {
        int rawResult;                  // Äîñòóï êàê ê ÷èñëó (äëÿ ñîâìåñòèìîñòè)
        GameResultType resultType;       // Òèïèçèðîâàííûé äîñòóï
        struct {
            unsigned int isWin : 1;        // Ôëàã ïîáåäû
            unsigned int isLose : 1;       // Ôëàã ïîðàæåíèÿ
            unsigned int isDraw : 1;       // Ôëàã íè÷üè
        };
    } gameResult;
    int winLineStartX;       // Íà÷àëî ëèíèè (êîîðäèíàòû êëåòêè)
    int winLineStartY;
    int winLineEndX;         // Êîíåö ëèíèè (êîîðäèíàòû êëåòêè)
    int winLineEndY;

    MoveLogger logger;  // Ëîããåð õîäîâ
    int showMoveLog;    // Ôëàã äëÿ îòîáðàæåíèÿ îêíà (1 - ïîêàçàòü, 0 - ñêðûòü)
} AppState;


// Ïðîòîòèïû ôóíêöèé äëÿ áîòîâ
void makeAIMoveEasy(AppState* state);
void makeAIMoveMedium(AppState* state);
void makeAIMoveHard(AppState* state);
void makeAIMoveExpert(AppState* state);





                                            /*                                     ÃÐÀÔÈÊÀ                                                */




// Ïðîòîòèï äëÿ ñîõðàíåíèé
void saveSettings(const GameSettings* settings);

// Ïðîòîòèï ÷åêîâ
int checkWinCondition(AppState* state, int symbol, int winLength);
int checkForDraw(AppState* state);

// Èíèöèàëèçàöèÿ ñîñòîÿíèÿ ïðèëîæåíèÿ
void initAppState(AppState* state) {
    // Íàñòðîéêè êàìåðû
    state->camera.zoom = 1.0f;
    state->camera.offsetX = 0.0f;
    state->camera.offsetY = 0.0f;

    // Ñîñòîÿíèå ìûøè
    state->mouse.lastX = 0.0;
    state->mouse.lastY = 0.0;
    state->mouse.isDragging = 0;

    // Èãðîâîå ïîëå
    state->grid.cells = NULL;
    state->grid.size = 0;
    state->grid.capacity = 0;

    // Âûáðàííàÿ êëåòêà
    state->selectedCellX = 0;
    state->selectedCellY = 0;

    // Ñîñòîÿíèå èíòåðôåéñà
    state->currentState = MENU_MAIN;
    state->showHelp = 0;
    state->menuSelectedItem = 0;
    state->settingsSelectedItem = 0;
    state->fontTexture = 0;
    state->fontSize = 32.0f;
    state->saveNotificationTimer = 0.0f;

    // Ëîããåð õîäîâ
    state->logger.head = NULL;
    state->logger.tail = NULL;
    state->logger.count = 0;
    state->showMoveLog = 0;  // Ïî óìîë÷àíèþ îêíî ñêðûòî

    
    
    // Íàñòðîéêè ïî óìîë÷àíèþ
    state->settings.difficulty = DIFFICULTY_EASY;
    state->settings.fieldSize = 0;      // Áåñêîíå÷íîå ïîëå
    state->settings.winLineLength = 3;  // Âûèãðûøíàÿ ëèíèÿ èç 3 ñèìâîëîâ
    state->settings.firstMove = FIRST_MOVE_PLAYER; // Ïî óìîë÷àíèþ èãðîê õîäèò ïåðâûì
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
        // Åñëè ôàéëà íåò, ñîçäàòü åãî ñ äåôîëòíûìè íàñòðîéêàìè
        saveSettings(&state->settings);
    }

    // Óñòàíîâêà òåêóùèõ íàñòðîéêè ðàâíûìè äåôîëòíûì
    state->defaultSettings = state->settings;

    state->gameResult.rawResult = 0;          // 0 - íåò ðåçóëüòàòà, 1 - ïîáåäà, -1 - ïîðàæåíèå
    state->winLineStartX = 0;       // Íà÷àëî ëèíèè (êîîðäèíàòû êëåòêè)
    state->winLineStartY = 0;
    state->winLineEndX = 0;         // Êîíåö ëèíèè (êîîðäèíàòû êëåòêè)
    state->winLineEndY = 0;
}



// Ñîõðàíåíèå íàñòðîåê â ôàéë
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

// Î÷èùåíèå ïîëÿ
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
        MoveLog* next = current->next;  // Êîïèðóåì next äî free
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
        // Óäàëÿåì ñàìûé ñòàðûé õîä (FIFO)
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


// Ôóíêöèÿ äîáàâëåíèÿ êëåòêè
void addCell(Grid* grid, int x, int y) {
    //  Ïðîâåðêà âõîäíîãî ïàðàìåòðà
    if (grid == NULL) return;

    //  Ïðîâåðêà íà äóáëèêàòû
    for (int i = 0; i < grid->size; ++i) {
        if (grid->cells[i].x == x && grid->cells[i].y == y) {
            return;
        }
    }

    //  Ðàñøèðåíèå ìàññèâà ïðè íåîáõîäèìîñòè
    if (grid->size >= grid->capacity) {
        const int newCapacity = (grid->capacity == 0) ? 4 : grid->capacity * 2;

        // Ñîçäàåì âðåìåííûé óêàçàòåëü äëÿ àíàëèçàòîðà
        Cell* const newCells = (Cell*)realloc(grid->cells, newCapacity * sizeof(Cell));
        if (!newCells) {
            fprintf(stderr, "Memory allocation failed\n");
            cleanupGrid(grid);
            exit(1);
        }

        grid->cells = newCells;
        grid->capacity = newCapacity;
    }

    //  Êîñòûëü äëÿ VS2019 - äåëàåì çàïèñü ÷åðåç óêàçàòåëü
    Cell* target = grid->cells + grid->size;
    target->x = x;
    target->y = y;
    target->symbol = 0;

    //  Óâåëè÷èâàåì ðàçìåð òîëüêî ïîñëå óñïåøíîé çàïèñè
    grid->size++;
}


// Èíèöèàëèçàöèÿ è ñîçäàíèå òåêñòà
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

    // Ñîçäàíèå òåêñòóðû
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


// Ôóíêöèÿ ñîõðàíåíèÿ èãðû
void saveGame(const Grid* grid, const GameSettings* settings) {
    FILE* file = fopen("saves.txt", "w");
    if (!file) {
        fprintf(stderr, "Incorrect file for saving\n");
        return;
    }

    // Ñîõðàíÿåì íàñòðîéêè ïåðâîé ñòðîêîé, äîáàâëÿåì firstMove
    fprintf(file, "SETTINGS %d %d %d %d\n",
        settings->difficulty,
        settings->fieldSize,
        settings->winLineLength,
        settings->firstMove);  // Äîáàâëÿåì ñîõðàíåíèå ïîðÿäêà õîäà

    // Çàòåì ñîõðàíÿåì êëåòêè
    for (int i = 0; i < grid->size; i++) {
        fprintf(file, "%d %d %d\n", grid->cells[i].x, grid->cells[i].y, grid->cells[i].symbol);
    }

    fclose(file);
}


// Çàãðóçêà èãðû
int loadGame(Grid* grid, GameSettings* settings) {
    FILE* file = fopen("saves.txt", "r");
    if (!file) {
        file = fopen("saves.txt", "w");
        if (file) fclose(file);
        return 0;
    }

    cleanupGrid(grid);

    // Íàñòðîéêè èç ïåðâîé ñòðîêè ôàéëà
    char line[256];
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SETTINGS", 8) == 0) {
            int temp1;
            int temp2;
            int check = sscanf(line + 8, "%d %d %d %d",
                &temp1,
                &settings->fieldSize,
                &settings->winLineLength,
                &temp2);  // Çàãðóæàåì ïîðÿäîê õîäà
            settings->difficulty = (GameDifficulty)temp1;
            settings->firstMove = (FirstMove)temp2;

            // Åñëè íå óäàëîñü ïðî÷èòàòü firstMove (ñòàðûé ôîðìàò ôàéëà)
            if (check < 4) {
                settings->firstMove = FIRST_MOVE_PLAYER; // Óñòàíàâëèâàåì ïî óìîë÷àíèþ
            }
        }
        else {
            // Åñëè ôàéë ñòàðîãî ôîðìàòà, ïåðåõîäèì â íà÷àëî
            fseek(file, 0, SEEK_SET);
            settings->firstMove = FIRST_MOVE_PLAYER; // Óñòàíàâëèâàåì ïî óìîë÷àíèþ
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


// Ðåíäåð òåêñòà
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

// Îòðèñîâêà óâåäîìëåíèÿ ñîõðàíåíèÿ
void drawSaveNotification(AppState* state, int width, int height) {
    if (state->saveNotificationTimer <= 0.0f) return;

    // Ñîõðàíÿåì òåêóùóþ ìàòðèöó ïðîåêöèè
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Ðàçìåðû è ïîçèöèÿ óâåäîìëåíèÿ
    float notifWidth = 300;
    float notifHeight = 60;
    float notifX = (width - notifWidth) / 2;
    float notifY = height - 100.0f;

    // Ðèñóåì ïîëóïðîçðà÷íûé ôîí
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.5f, 0.1f, 0.9f * (state->saveNotificationTimer / 1.0f)); // Ïëàâíîå èñ÷åçíîâåíèå
    glBegin(GL_QUADS);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();
    glDisable(GL_BLEND);

    // Ðàìêà
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

    // Òåêñò óâåäîìëåíèÿ
    float textAlpha = state->saveNotificationTimer / 1.0f;
    renderText(state, "Game saved successfully!",
        notifX + 20, notifY + 35,
        0.7f, 1.0f, 1.0f, 1.0f, textAlpha); 

    // Âîññòàíàâëèâàåì ìàòðèöû
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


// Îòðèñîâêà ãëàâíîãî ìåíþ
void drawMainMenu(AppState* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Ôîí ìåíþ
    glColor3f(0.1f, 0.1f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // Çàãîëîâîê
    float titleWidth = 0;
    const char* title = "Krestiki-Noliki";
    const char* p = title;
    while (*p) {
        
            titleWidth += state->cdata[*p - 32].xadvance;
        
        p++;
    }
    renderText(state, title, (1240 - titleWidth * 1.5f) / 2, 1000, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Êíîïêè ìåíþ
    const char* items[] = { "New Game", "Load Game", "Settings", "About" };
    for (int i = 0; i < 4; i++) {
        float x = 500;
        float y = 800.0f - i * 200.0f;
        float width = 240;
        float height = 80;

        // Ðàìêà êíîïêè
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();

        // Çàëèâêà êíîïêè
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

        // Òåêñò êíîïêè
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

// Îòðèñîâêà ìåíþ îá àâòîðàõ
void drawAboutScreen(AppState* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Ôîí
    glColor3f(0.1f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // Çàãîëîâîê
    float titleWidth = 0;
    const char* title = "About";
    const char* p = title;
    while (*p) {
        
            titleWidth += state->cdata[*p - 32].xadvance;
        
        p++;
    }
    renderText(state, title, (1240 - titleWidth * 1.5f) / 2, 1000, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Òåêñò
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


//  ôóíêöèÿ îòðèñîâêè ýêðàíà íàñòðîåê
void drawSettingsScreen(AppState* state, GLFWwindow* window) {
    // Ïîëó÷àåì êîîðäèíàòû ìûøè
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseY = height - mouseY; // Èíâåðòèðóåì Y
    // Ìàñøòàáèðóåì ê âèðòóàëüíûì êîîðäèíàòàì 1240x1240
    mouseX = (mouseX / width) * 1240.0f;
    mouseY = (mouseY / height) * 1240.0f;

    // Óñòàíîâêà ìàòðèöû ïðîåêöèè
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Ôîí
    glColor3f(0.2f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1240, 0);
    glVertex2f(1240, 1240);
    glVertex2f(0, 1240);
    glEnd();

    // Çàãîëîâîê
    renderText(state, "Settings", 500, 1100, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Êîîðäèíàòû è ðàçìåðû ýëåìåíòîâ
    float yPos = 800;
    float buttonWidth = 120;
    float buttonHeight = 50;
    float arrowSize = 20;
    float valueBoxX = 700;

    //  Íàñòðîéêà ñëîæíîñòè
    renderText(state, "Difficulty Level:", 300.0f, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    const char* difficultyOptions[] = { "Easy", "Medium", "Hard", "Expert" };
    for (int i = 0; i < 4; i++) {
        float xPos = 600.0f + i * 130.0f;

        // Ïðîâåðêà íàâåäåíèÿ ìûøè
        int isHovered = (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

        // Öâåò êíîïêè
        if ((int)state->settings.difficulty == i) {
            glColor3f(0.4f, 0.4f, 0.8f); // Âûáðàííûé âàðèàíò
        }
        else if (isHovered) {
            glColor3f(0.3f, 0.3f, 0.6f); // Íàâåäåíèå
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f); // Îáû÷íûé
        }

        // Ðèñóåì êíîïêó
        glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Ðàìêà
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Òåêñò
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

    //  Ðàçìåð ïîëÿ
    yPos -= 120;
    renderText(state, "Field Size (0=infinite):", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    char fieldSizeText[32];
    snprintf(fieldSizeText, sizeof(fieldSizeText), "%d", state->settings.fieldSize);

    // Ïðîâåðêà íàâåäåíèÿ íà êíîïêè
    int decHovered = (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);
    int incHovered = (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

    // Êíîïêà óìåíüøåíèÿ
    glColor3f(decHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX - 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Ïîëå çíà÷åíèÿ
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Êíîïêà óâåëè÷åíèÿ
    glColor3f(incHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX + buttonWidth + 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Ðàìêà
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Òåêñò çíà÷åíèÿ
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

    //  Äëèíà âûèãðûøíîé ëèíèè
    yPos -= 120;
    renderText(state, "Win Line Length:", 300, yPos, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    char winLineText[32];
    snprintf(winLineText, sizeof(winLineText), "%d", state->settings.winLineLength);

    // Ïðîâåðêà íàâåäåíèÿ íà êíîïêè
    decHovered = (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);
    incHovered = (mouseX >= (double)valueBoxX + buttonWidth + 10 && mouseX <= (double)valueBoxX + buttonWidth + 40 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

    // Êíîïêà óìåíüøåíèÿ
    glColor3f(decHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX - 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX - 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Ïîëå çíà÷åíèÿ
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Êíîïêà óâåëè÷åíèÿ
    glColor3f(incHovered ? 0.4f : 0.3f, 0.3f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(valueBoxX + buttonWidth + 25, yPos + buttonHeight / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 - arrowSize / 2);
    glVertex2f(valueBoxX + buttonWidth + 10, yPos + buttonHeight / 2 + arrowSize / 2);
    glEnd();

    // Ðàìêà
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(valueBoxX, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos);
    glVertex2f(valueBoxX + buttonWidth, yPos + buttonHeight);
    glVertex2f(valueBoxX, yPos + buttonHeight);
    glEnd();

    // Òåêñò çíà÷åíèÿ
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

        // Ïðîâåðêà íàâåäåíèÿ ìûøè
        int isHovered = (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight);

        // Öâåò êíîïêè
        if ((int)state->settings.firstMove == i) {
            glColor3f(0.4f, 0.4f, 0.8f); // Âûáðàííûé âàðèàíò
        }
        else if (isHovered) {
            glColor3f(0.3f, 0.3f, 0.6f); // Íàâåäåíèå
        }
        else {
            glColor3f(0.2f, 0.2f, 0.4f); // Îáû÷íûé
        }

        // Ðèñóåì êíîïêó
        glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Ðàìêà
        glColor3f(0.8f, 0.8f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + buttonWidth, yPos);
        glVertex2f(xPos + buttonWidth, yPos + buttonHeight);
        glVertex2f(xPos, yPos + buttonHeight);
        glEnd();

        // Òåêñò
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

    //  Êíîïêà ñîõðàíåíèÿ
    yPos -= 150;
    float saveX = (1240 - 300) / 2;
    int saveHovered = (mouseX >= saveX && mouseX <= (double)saveX + 300 &&
        mouseY >= yPos && mouseY <= (double)yPos + 60);

    // Ôîí êíîïêè
    glColor3f(saveHovered ? 0.4f : 0.3f, saveHovered ? 0.8f : 0.6f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(saveX, yPos);
    glVertex2f(saveX + 300, yPos);
    glVertex2f(saveX + 300, yPos + 60);
    glVertex2f(saveX, yPos + 60);
    glEnd();

    // Ðàìêà
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(saveX, yPos);
    glVertex2f(saveX + 300, yPos);
    glVertex2f(saveX + 300, yPos + 60);
    glVertex2f(saveX, yPos + 60);
    glEnd();

    // Òåêñò
    renderText(state, "Save Settings", saveX + 70, yPos + 30, 0.9f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Êíîïêà íàçàä
    renderText(state, "Back (ESC)", 50, 50, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
}


// Ïîëíàÿ ôóíêöèÿ îáðàáîòêè êëèêîâ â íàñòðîéêàõ
void handleSettingsClick(AppState* state, GLFWwindow* window, int button) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    // Ïîëó÷àåì êîîðäèíàòû ìûøè
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseY = height - mouseY; // Èíâåðòèðóåì Y
    // Ìàñøòàáèðóåì ê âèðòóàëüíûì êîîðäèíàòàì 1240x1240
    mouseX = (mouseX / width) * 1240.0f;
    mouseY = (mouseY / height) * 1240.0f;

    // Êîîðäèíàòû ýëåìåíòîâ
    float yPos = 800;
    float buttonWidth = 120;
    float buttonHeight = 50;
    float valueBoxX = 700;

    //  Ïðîâåðêà êëèêîâ ïî óðîâíþ ñëîæíîñòè
    for (int i = 0; i < 4; i++) {
        float xPos = 600.0f + i * 130.0f;
        if (mouseX >= xPos && mouseX <= (double)xPos + buttonWidth &&
            mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
            state->settings.difficulty = i;
            return;
        }
    }

    //  Ïðîâåðêà êëèêîâ ïî ðàçìåðó ïîëÿ
    yPos -= 120;

    // Êíîïêà óìåíüøåíèÿ
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

    // Êíîïêà óâåëè÷åíèÿ
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

    //  Ïðîâåðêà êëèêîâ ïî äëèíå ëèíèè
    yPos -= 120;

    // Êíîïêà óìåíüøåíèÿ
    if (mouseX >= (double)valueBoxX - 40 && mouseX <= (double)valueBoxX - 10 &&
        mouseY >= yPos && mouseY <= (double)yPos + buttonHeight) {
        state->settings.winLineLength = (state->settings.winLineLength > 3) ? state->settings.winLineLength - 1 : 3;
        return;
    }

    // Êíîïêà óâåëè÷åíèÿ
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
    

    //  Ïðîâåðêà êëèêà ïî êíîïêå ñîõðàíåíèÿ
    yPos -= 150;
    float saveX = (1240 - 300) / 2;
    if (mouseX >= saveX && mouseX <= (double)saveX + 300 &&
        mouseY >= yPos && mouseY <= (double)yPos + 60) {
        saveSettings(&state->settings);
        state->defaultSettings = state->settings; // Îáíîâëÿåì äåôîëòíûå íàñòðîéêè
        return;
    }

    //  Ïðîâåðêà êëèêà ïî êíîïêå íàçàä (âåðõíèé ëåâûé óãîë)
    if (mouseX < 200 && mouseY < 100) {
        state->currentState = MENU_MAIN;
    }
}


// Çàìåíà âèíäîâñêîé ôóíêöèè _itoa ò.ê. äëÿ ëèíóõà íèçÿ
static void int_to_str(int value, char* str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char* ptr = str;
    int is_negative = 0;

    // Îáðàáîòêà îòðèöàòåëüíûõ ÷èñåë äëÿ base 10
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    // Îáðàáàòûâàåì 0 ÿâíî, èíà÷å áóäåò ïóñòàÿ ñòðîêà
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    // Çàïèñûâàåì öèôðû â îáðàòíîì ïîðÿäêå
    char* start = ptr;
    while (value != 0) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
        value /= base;
    }

    // Äîáàâëÿåì çíàê ìèíóñ äëÿ îòðèöàòåëüíûõ ÷èñåë
    if (is_negative) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    // Ðàçâîðà÷èâàåì ñòðîêó
    ptr--;
    while (start < ptr) {
        char tmp = *start;
        *start++ = *ptr;
        *ptr-- = tmp;
    }
}


// Îòðèñîâêà îêíà ïîìîùè
void drawHelpWindow(AppState* state) {
    // Ñîõðàíÿåì òåêóùóþ ìàòðèöó ïðîåêöèè
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Ðèñóåì ïîëóïðîçðà÷íûé ôîí
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

    // Ðàìêà
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(300, 170);
    glVertex2f(940, 170);
    glVertex2f(940, 900);
    glVertex2f(300, 900);
    glEnd();

    // Çàãîëîâîê
    renderText(state, "Game Controls", 450, 850, 1.2f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Òåêñò èíñòðóêöèé
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

    // Ïîëó÷àåì òåêñòîâûå ïðåäñòàâëåíèÿ íàñòðîåê
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

    // Îòðèñîâûâàåì íàñòðîéêè
    renderText(state, instructions[6], 320, 450, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Ñëîæíîñòü
    float x = 320;
    float y = 400;
    renderText(state, instructions[7], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, difficultyNames[state->settings.difficulty], x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Ðàçìåð ïîëÿ
    y -= 50;
    renderText(state, instructions[8], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, fieldSizeText, x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Äëèíà ëèíèè
    y -= 50;
    renderText(state, instructions[9], x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderText(state, winLineText, x + 200, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Îñòàëüíîé òåêñò
    renderText(state, instructions[11], 320, 200, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Âîññòàíàâëèâàåì ìàòðèöû
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void drawMoveLog(AppState* state) {
    // 1. Íàñòðîéêà ìàòðèö è ïðîçðà÷íîñòè
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1240, 0, 1240, -1, 1); // Ðàçìåð îêíà 1240x1240
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 2. Ïàðàìåòðû îêíà
    float x = 50.0f, y = 100.0f;
    float width = 300.0f, height = 200.0f;

    // 3. Ôîí îêíà (ïîëóïðîçðà÷íûé)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.1f, 0.2f, 0.9f); // Òåìíî-ñèíèé ñ ïðîçðà÷íîñòüþ
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 4. Ðàìêà îêíà
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f); // Áåëàÿ
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 5. Òåêñò (çàãîëîâîê)
    renderText(state, "Last Moves:", x + 10.0f, y + height - 30.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f);

    // 6. Ñïèñîê õîäîâ
    MoveLog* current = state->logger.head;
    float textY = y + height - 60.0f; // Ñòàðòîâàÿ ïîçèöèÿ äëÿ òåêñòà
    int count = 0;

    while (current != NULL && count < 6) {
        char buf[50];
        const char* type = (current->type == MOVE_PLAYER) ? "Player" : "AI";
        snprintf(buf, sizeof(buf), "%s: (%d, %d)", type, current->x, current->y);

        // Ïðîâåðÿåì, íå âûøëè ëè çà ãðàíèöû îêíà
        if (textY < y + 10.0f) break;

        renderText(state, buf, x + 10.0f, textY, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f);
        textY -= 25.0f; // Ñìåùàåì âíèç
        current = current->next;
        count++;
    }

    // 7. Âîññòàíàâëèâàåì ìàòðèöû
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_BLEND);
}


// Ïîäñêàçêà â ïðàâîì âåðõíåì óãëó î ïîìîùè
void drawHelpHint(AppState* state, int width, int height) {
    // Ñîõðàíÿåì òåêóùóþ ìàòðèöó ïðîåêöèè
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Ðàçìåðû è ïîçèöèÿ áëîêà ïîäñêàçêè
    float hintWidth = 200;
    float hintHeight = 50;
    float hintX = width - hintWidth - 20;
    float hintY = height - hintHeight - 20;

    // Ðèñóåì ïîëóïðîçðà÷íûé ôîí
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

    // Ðàìêà
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(hintX, hintY);
    glVertex2f(hintX + hintWidth, hintY);
    glVertex2f(hintX + hintWidth, hintY + hintHeight);
    glVertex2f(hintX, hintY + hintHeight);
    glEnd();

    // Òåêñò ïîäñêàçêè
    renderText(state, "Press 'H' for Help", hintX + 10, hintY + 25, 0.6f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Âîññòàíàâëèâàåì ìàòðèöû
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Îòðèñîâêà êëåòî÷íîãî ïîëÿ
void drawGrid(float visibleLeft, float visibleRight,
    float visibleBottom, float visibleTop, float zoom, int fieldSize) {
    // Ðàçìåð êëåòêè â ìèðîâûõ êîîðäèíàòàõ
    float cellSize = 2.0f / (10.0f * zoom);

    // Îïðåäåëÿåì ãðàíèöû âèäèìîé îáëàñòè â êëåòêàõ
    int startX = (int)(visibleLeft / cellSize) - 1;
    int endX = (int)(visibleRight / cellSize) + 1;
    int startY = (int)(visibleBottom / cellSize) - 1;
    int endY = (int)(visibleTop / cellSize) + 1;

    // Åñëè ïîëå îãðàíè÷åíî, êîððåêòèðóåì ãðàíèöû îòðèñîâêè
    if (fieldSize > 0) {
        startX = (startX < -fieldSize / 2) ? -fieldSize / 2 : startX;
        endX = (endX > fieldSize / 2) ? fieldSize / 2 : endX;
        startY = (startY < -fieldSize / 2) ? -fieldSize / 2 : startY;
        endY = (endY > fieldSize / 2) ? fieldSize / 2 : endY;
    }

    // Îòðèñîâûâàåì òîëüêî âèäèìûå êëåòêè
    for (int x = startX; x <= endX; ++x) {
        for (int y = startY; y <= endY; ++y) {
            // Åñëè ïîëå îãðàíè÷åíî, ïðîïóñêàåì êëåòêè çà ãðàíèöàìè
            if (fieldSize > 0 && (abs(x) > fieldSize / 2 || abs(y) > fieldSize / 2)) {
                continue;
            }

            // Âû÷èñëÿåì êîîðäèíàòû óãëîâ êëåòêè
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

// Îòðèñîâêà ïîäñâåòêè âûáðàííîé êëåòêè
void drawSelectedCell(int x, int y, float cellSize) {
    // Âû÷èñëÿåì êîîðäèíàòû óãëîâ âûáðàííîé êëåòêè
    float x1 = x * cellSize;
    float y1 = y * cellSize;
    float x2 = x1 + cellSize;
    float y2 = y1 + cellSize;
    glColor3f(1.0, 1.0, 0.0);

    // Îòðèñîâûâàåì ïîäñâåòêó
    glLineWidth(5.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// Îòðèñîâêà êðåñòèêà â êëåòêå
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

// Îòðèñîâêà íîëèêà â êëåòêå
void drawCircle(float x1, float y1, float x2, float y2) {
    glColor3f(0.0, 0.0, 1.0);  // Ñèíèé öâåò äëÿ íîëèêà

    float centerX = (x1 + x2) / 2.0f;
    float centerY = (y1 + y2) / 2.0f;
    float outerRadius = (x2 - x1) / 2.5f;  // Âíåøíèé ðàäèóñ
    float innerRadius = outerRadius * 0.7f; // Âíóòðåííèé ðàäèóñ äëÿ òîëùèíû ëèíèè

    // Ðèñóåì íîëèê ñ ïîìîùüþ òðåóãîëüíèêîâ
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 360; i += 5) {  // Øàã 5 ãðàäóñîâ ìîæíî óìåíüøèòü äëÿ áîëüøåé ãëàäêîñòè
        float angle = i * 3.14159f / 180.0f;

        // Âíåøíÿÿ òî÷êà
        glVertex2f((GLfloat)centerX + (GLfloat)outerRadius * (GLfloat)cos(angle),
            (GLfloat)centerY + (GLfloat)outerRadius * (GLfloat)sin(angle));

        // Âíóòðåííÿÿ òî÷êà
        glVertex2f((GLfloat)centerX + (GLfloat)innerRadius * (GLfloat)cos(angle),
            (GLfloat)centerY + (GLfloat)innerRadius * (GLfloat)sin(angle));
    }
    glEnd();
}

// Îáðàáîòêà çóìà êîëåñîì ìûøè
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

    // Îãðàíè÷åíèå çóìà
    if (state->camera.zoom < 0.4f) state->camera.zoom = 0.4f;
    if (state->camera.zoom > 3.0f) state->camera.zoom = 3.0f;
}


// Îáðàáîòêà íàæàòèé êëàâèø ìûøè
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS && state->currentState == MENU_SETTINGS) {
        handleSettingsClick(state, window, button);
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (state->currentState == MENU_MAIN) {
            // Îáðàáîòêà êëèêîâ è íàâåäåíèÿ â ìåíþ
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (mods == 1) {
                int kostil = 0;
                kostil++;
            }
            // Ïðåîáðàçóåì êîîðäèíàòû ìûøè â êîîðäèíàòû ýêðàíà (1240x1240)
            ypos = height - ypos; // Èíâåðòèðóåì Y
            float menuX = ((float)xpos / (float)width) * 1240.0f;
            float menuY = ((float)ypos / (float)height) * 1240.0f;

            // Ïðîâåðÿåì, êàêàÿ êíîïêà ïîä êóðñîðîì
            if (menuX >= 500 && menuX <= 740) { // Øèðèíà êíîïêè 240
                if (menuY >= 800 && menuY <= 880) { // New Game (ïåðâàÿ êíîïêà)
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
                        
                        // Åñëè áîò õîäèò ïåðâûì, äåëàåì åãî õîä ñðàçó
                        if (state->settings.firstMove == FIRST_MOVE_AI) {
                            // Ñïåöèàëüíàÿ ëîãèêà äëÿ ïåðâîãî õîäà áîòà
                                addCell(&state->grid, 0, 0);
                                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                                logMove(&state->logger, 0, 0, MOVE_AI);
                        }

                    }
                }
                else if (menuY >= 600 && menuY <= 680) { // Load Game (âòîðàÿ êíîïêà)
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
                else if (menuY >= 400 && menuY <= 480) { // Settings (òðåòüÿ êíîïêà)
                    state->menuSelectedItem = 2;
                    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                        state->currentState = MENU_SETTINGS;
                    }
                }
                else if (menuY >= 200 && menuY <= 280) { // About (÷åòâåðòàÿ êíîïêà)
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

// Îáðàáîòêà ïåðåìåùåíèÿ êóðñîðà
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
        // Îáðàáîòêà íàâåäåíèÿ â ãëàâíîì ìåíþ
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ypos = height - ypos; // Èíâåðòèðóåì Y
        float menuX = ((float)xpos / (float)width) * 1240.0f;
        float menuY = ((float)ypos / (float)height) * 1240.0f;

        // Ïðîâåðÿåì, êàêàÿ êíîïêà ïîä êóðñîðîì
        if (menuX >= 500 && menuX <= 740) { // Øèðèíà êíîïêè 240
            if (menuY >= 800 && menuY <= 880) { // New Game (ïåðâàÿ êíîïêà)
                
                state->menuSelectedItem = 0;

            }
            else if (menuY >= 600 && menuY <= 680) { // Load Game (âòîðàÿ êíîïêà)
                state->menuSelectedItem = 1;
            }
            else if (menuY >= 400 && menuY <= 480) { // Settings (òðåòüÿ êíîïêà)
                state->menuSelectedItem = 2;
            }
            else if (menuY >= 200 && menuY <= 280) { // About (÷åòâåðòàÿ êíîïêà)
                state->menuSelectedItem = 3;
            }
        }
    }
}
// Ïðîâåðêà íè÷üè
int checkForDraw(AppState* state) {
    if (state->settings.fieldSize <= 0) return 0; // Áåñêîíå÷íîå ïîëå - íè÷üÿ íåâîçìîæíà

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
                return 0; // Íàøëè ïóñòóþ êëåòêó - íè÷üè íåò
            }
        }
    }
    return 1; // Âñå êëåòêè çàïîëíåíû - íè÷üÿ
}

// Îáðàáîòêà íàæàòèÿ êëàâèø
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
            state->menuSelectedItem = (state->menuSelectedItem - 1 + 4) % 4; // Ïî æåëàíèþ â áóäóùåì çàìåèòü çíà÷åíèÿ menuSelectedItem íà enum
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
                // Èñïîëüçóåì íàñòðîéêè èç defaultSettings
                state->settings = state->defaultSettings;
                state->currentState = MENU_GAME;
                state->gameResult.rawResult = 0;

                // Åñëè áîò õîäèò ïåðâûì, äåëàåì åãî õîä ñðàçó
                if (state->settings.firstMove == FIRST_MOVE_AI) {
                    // Ñïåöèàëüíàÿ ëîãèêà äëÿ ïåðâîãî õîäà áîòà
                    if (state->settings.fieldSize > 0) {
                        // Äëÿ îãðàíè÷åííîãî ïîëÿ - ñòàâèì â öåíòð
                        addCell(&state->grid, 0, 0);
                        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                        logMove(&state->logger, 0, 0, MOVE_AI);
                    }
                    else {
                        // Äëÿ áåñêîíå÷íîãî ïîëÿ - ñòàâèì â (0,0)
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
                    state->gameResult.rawResult = 0; // Ñáðàñûâàåì ñîñòîÿíèå èãðû
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
                state->gameResult.rawResult = 0; // Ñáðàñûâàåì ñîñòîÿíèå èãðû ïðè âûõîäå
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
            if (state->gameResult.rawResult != 0) break; // Èãðà óæå çàâåðøåíà

            // Îïðåäåëÿåì ñèìâîë èãðîêà è áîòà â çàâèñèìîñòè îò òîãî, êòî õîäèò ïåðâûì
            int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
            aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;

            // Ïðîâåðÿåì, íå çàíÿòà ëè êëåòêà
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

            // Äåëàåì õîä èãðîêà
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

            // Ïðîâåðêà ïîáåäû ïîñëå õîäà
            if (checkWinCondition(state, playerSymbol, state->settings.winLineLength)) {
                state->gameResult.isWin = 1;
                break;
            }

            // Ïðîâåðêà íè÷üè
            if (checkForDraw(state)) {
                state->gameResult.isDraw = 1;
                break;
            }

            // Õîä áîòà (òîëüêî åñëè èãðà íå çàâåðøåíà)
            if (state->gameResult.rawResult == 0) {
                // Îïðåäåëÿåì ôóíêöèþ äëÿ õîäà áîòà â çàâèñèìîñòè îò ñëîæíîñòè
                void (*aiMoveFunc)(AppState*) = NULL;
                switch (state->settings.difficulty) {
                case DIFFICULTY_EASY: aiMoveFunc = makeAIMoveEasy; break;
                case DIFFICULTY_MEDIUM: aiMoveFunc = makeAIMoveMedium; break;
                case DIFFICULTY_HARD: aiMoveFunc = makeAIMoveHard; break;
                case DIFFICULTY_EXPERT: aiMoveFunc = makeAIMoveExpert; break;
                }

                if (aiMoveFunc) {
                    aiMoveFunc(state);

                    // Ïðîâåðêà ïîáåäû áîòà ïîñëå åãî õîäà
                    if (checkWinCondition(state, aiSymbol, state->settings.winLineLength)) {
                        state->gameResult.isLose = 1;
                    }
                    // Ïðîâåðêà íè÷üè
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
                state->saveNotificationTimer = 1.0; // Óñòàíàâëèâàåì òàéìåð íà 2 ñåêóíäû
                break;
            }
            break;
        case GLFW_KEY_ESCAPE:
            if (state->currentState == MENU_GAME) {
                // Âîññòàíàâëèâàåì íàñòðîéêè èç defaultSettings
                cleanupMoveLogger(&state->logger);
                state->settings = state->defaultSettings;
                state->currentState = MENU_MAIN;
                state->gameResult.rawResult = 0;
            }
            break;
        case GLFW_KEY_H:
            state->showHelp = !state->showHelp; // Ïåðåêëþ÷àåì îòîáðàæåíèå ïîìîùè
            break;
        case GLFW_KEY_M:  // M - ïîêàçàòü/ñêðûòü ëîã
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

// Îòðèñîâêà ïîáåäíãî ñîîáùåíèÿ
void drawWinLine(AppState* state, int width, int height) {
    if (state->gameResult.rawResult == 0) return;

    // Ñîõðàíÿåì ìàòðèöû
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Ðàçìåðû è ïîçèöèÿ óâåäîìëåíèÿ
    float notifWidth = 400;
    float notifHeight = 100;
    float notifX = (width - notifWidth) / 2;
    float notifY = height - 150.0f;

    // Ðèñóåì ïîëóïðîçðà÷íûé ôîí
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (state->gameResult.isWin == 1) { // Ïîáåäà
        glColor4f(0.1f, 0.5f, 0.1f, 0.9f);
    }
    else if (state->gameResult.isLose == 1) { // Ïîðàæåíèå
        glColor4f(0.5f, 0.1f, 0.1f, 0.9f);
    }
    else { // Íè÷üÿ
        glColor4f(0.3f, 0.3f, 0.3f, 0.9f);
    }

    glBegin(GL_QUADS);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();

    // Ðàìêà
    glColor4f(0.8f, 0.8f, 0.0f, 0.9f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(notifX, notifY);
    glVertex2f(notifX + notifWidth, notifY);
    glVertex2f(notifX + notifWidth, notifY + notifHeight);
    glVertex2f(notifX, notifY + notifHeight);
    glEnd();

    // Òåêñò óâåäîìëåíèÿ
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

    // Âîññòàíàâëèâàåì ìàòðèöû
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Îòðèñîâêà ïîáåäíîé ëèíèè
void drawWinningLine(AppState* state) {

    // Îïðåäåëÿåì, êòî âûèãðàë
    int winnerSymbol = 0;
    if (state->gameResult.isWin) {
        winnerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    }
    else if (state->gameResult.isLose) {
        winnerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    }

    // Óñòàíàâëèâàåì öâåò â çàâèñèìîñòè îò òîãî, êòî âûèãðàë
    if (winnerSymbol == 1) { // Êðåñòèê
        int isPlayer = (state->settings.firstMove == FIRST_MOVE_PLAYER);
        if (isPlayer) {
            glColor3f(1.0f, 0.0f, 0.0f); // Êðàñíûé äëÿ èãðîêà
        }
        else {
            glColor3f(0.5f, 0.0f, 0.5f); // Ôèîëåòîâûé äëÿ áîòà
        }
    }
    else { // Íîëèê
        int isPlayer = (state->settings.firstMove != FIRST_MOVE_PLAYER);
        if (isPlayer) {
            glColor3f(0.0f, 0.0f, 1.0f); // Ñèíèé äëÿ èãðîêà
        }
        else {
            glColor3f(0.0f, 1.0f, 1.0f); // Ãîëóáîé äëÿ áîòà
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Ðèñóåì ëèíèþ ìåæäó öåíòðàìè êëåòîê
    glColor3f(1.0f, 1.0f, 0.0f); // Æåëòûé öâåò
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







                                                    /*                         ÀËÃÎÐÈÒÌÛ                                  */


// Ïðîâåðêà, åñòü ëè âûèãðûøíàÿ ëèíèÿ èç symbol çàäàííîé äëèíû
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

            // Ïðîâåðÿåì â îäíîì íàïðàâëåíèè
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

            // Ïðîâåðÿåì â ïðîòèâîïîëîæíîì íàïðàâëåíèè
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



// Ôóíêöèÿ äëÿ õîäà áîòà (ëåãêèé óðîâåíü), óñëîâíî ðàíäîì â ðàäèóñå äëèíû ïîáåäíîé ëèíèè
void makeAIMoveEasy(AppState* state) {
    
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }


    // Ñîáèðàåì âñå èãðîêà
    int crossCount = 0;
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) crossCount++;
    }

    

    // Âûáèðàåì ñëó÷àéíûé
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

    // Ïûòàåìñÿ ïîñòàâèòü ðÿäîì ñ âûáðàííûì êðåñòèêîì
    int attempts = 0;
    const int maxAttempts = state->settings.winLineLength;

    while (attempts < maxAttempts) {
        // Âûáèðàåì ñëó÷àéíîå íàïðàâëåíèå è ðàññòîÿíèå (íå áîëüøå winLineLength)
        int dx = (rand() % (2 * state->settings.winLineLength + 1)) - state->settings.winLineLength;
        int dy = (rand() % (2 * state->settings.winLineLength + 1)) - state->settings.winLineLength;

        // Óáåäèìñÿ, ÷òî ìû íå îñòàëèñü íà ìåñòå
        if (dx == 0 && dy == 0) continue;

        int newX = targetX + dx;
        int newY = targetY + dy;

        // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà è â ïðåäåëàõ ïîëÿ (åñëè ïîëå îãðàíè÷åíî)
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
                // Ïðîâåðÿåì, ÷òî â ïðåäåëàõ îãðàíè÷åííîãî ïîëÿ
                if (abs(newX) <= state->settings.fieldSize / 2 &&
                    abs(newY) <= state->settings.fieldSize / 2) {
                    addCell(&state->grid, newX, newY);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                    logMove(&state->logger, newX, newY, MOVE_AI);
                    return;
                }
            }
            else {
                // Äëÿ áåñêîíå÷íîãî ïîëÿ ïðîñòî äîáàâëÿåì
                addCell(&state->grid, newX, newY);
                state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
                logMove(&state->logger, newX, newY, MOVE_AI);
                return;
            }
        }

        attempts++;
    }

    // Åñëè íå óäàëîñü íàéòè ìåñòî ðÿäîì, ñòàâèì â ñëó÷àéíóþ ñâîáîäíóþ êëåòêó
    if (state->settings.fieldSize > 0) {
        // Äëÿ îãðàíè÷åííîãî ïîëÿ
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

// Áîò ñðåäíåé ñëîæíîñòè ìåíüøå ðàíäîìà, âèäèò î÷åâèäíûå óãðîçû
void makeAIMoveMedium(AppState* state) {
    

    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. Ïðîâåðèòü, åñòü ëè âûèãðûøíûé õîä äëÿ áîòà (íîëèêà)
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Èùåì ñâîè íîëèêè
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // Ïðîâåðÿåì âñå íàïðàâëåíèÿ
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí íîëèê
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
                                    found = -1; // Êðåñòèê ìåøàåò
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Ïóñòàÿ êëåòêà
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Êðåñòèê íà ïóòè
                        }
                    }

                    // Åñëè íàøëè âûèãðûøíûé õîä (ðîâíî îäíà ïóñòàÿ êëåòêà â ëèíèè íóæíîé äëèíû)
                    if (count == state->settings.winLineLength - 1 && emptyCount == 1) {
                        // Ïðîâåðÿåì, ÷òî êëåòêà â ïðåäåëàõ ïîëÿ
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

    // 2. Ïðîâåðèòü, íóæíî ëè áëîêèðîâàòü âûèãðûøíûé õîä èãðîêà
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Èùåì êðåñòèêè èãðîêà
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí êðåñòèê
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
                                    found = -1; // Íîëèê ìåøàåò
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Ïóñòàÿ êëåòêà
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Íîëèê íà ïóòè
                        }
                    }

                    // Åñëè íóæíî áëîêèðîâàòü (ðîâíî îäíà ïóñòàÿ êëåòêà â ëèíèè íóæíîé äëèíû)
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

    // 3. Çàíÿòü öåíòð, åñëè îí ñâîáîäåí
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

    // 4. Ñëó÷àéíûé âûáîð ìåæäó àòàêîé è çàùèòîé (óïðîùåííàÿ âåðñèÿ)
    int strategy = rand() % 2; // 0 - àòàêà, 1 - çàùèòà

    if (strategy == 0) { // Àòàêà - ïîñòàâèòü ðÿäîì ñî ñâîèì íîëèêîì
        // Ñîáèðàåì âñå ñâîè íîëèêè
        int nolikCount = 0;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].symbol == aiSymbol) nolikCount++;
        }

        if (nolikCount > 0) {
            // Âûáèðàåì ñëó÷àéíûé íîëèê
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

            // Ïûòàåìñÿ ïîñòàâèòü ðÿäîì (â ðàäèóñå 1 êëåòêè)
            for (int attempt = 0; attempt < 8; attempt++) {
                int dx = (rand() % 3) - 1; // -1, 0 èëè 1
                int dy = (rand() % 3) - 1;

                if (dx == 0 && dy == 0) continue;

                int newX = targetX + dx;
                int newY = targetY + dy;

                // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == newX &&
                        state->grid.cells[i].y == newY) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Ïðîâåðÿåì ãðàíèöû ïîëÿ
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
    else { // Çàùèòà - ïîñòàâèòü ðÿäîì ñ êðåñòèêîì
        // Ñîáèðàåì âñå êðåñòèêè
        int krestikCount = 0;
        for (int i = 0; i < state->grid.size; i++) {
            if (state->grid.cells[i].symbol == playerSymbol) krestikCount++;
        }

        if (krestikCount > 0) {
            // Âûáèðàåì ñëó÷àéíûé êðåñòèê
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

            // Ïûòàåìñÿ ïîñòàâèòü ðÿäîì (â ðàäèóñå 1 êëåòêè)
            for (int attempt = 0; attempt < 8; attempt++) {
                int dx = (rand() % 3) - 1;
                int dy = (rand() % 3) - 1;

                if (dx == 0 && dy == 0) continue;

                int newX = targetX + dx;
                int newY = targetY + dy;

                // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == newX &&
                        state->grid.cells[i].y == newY) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Ïðîâåðÿåì ãðàíèöû ïîëÿ
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

    // Åñëè âñå ñòðàòåãèè íå ñðàáîòàëè, äåëàåì ñëó÷àéíûé õîä
    makeAIMoveEasy(state);
}


// Áîò ñëîæíîãî óðîâíÿ, ðàáîòàåò íà ýâðèñòèêàõ, äîâîëüíî ýôôåêòèâåí, ïîäðîáíåå â îò÷åòå
void makeAIMoveHard(AppState* state) {
    

    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;
    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. Ïðîâåðèòü, åñòü ëè âûèãðûøíûé õîä äëÿ áîòà (íîëèêà)
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Èùåì ñâîè íîëèêè
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // Ïðîâåðÿåì âñå íàïðàâëåíèÿ
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí íîëèê
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
                                    found = -1; // Êðåñòèê ìåøàåò
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Ïóñòàÿ êëåòêà
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Êðåñòèê íà ïóòè
                        }
                    }

                    // Åñëè íàøëè âûèãðûøíûé õîä (ðîâíî îäíà ïóñòàÿ êëåòêà â ëèíèè íóæíîé äëèíû)
                    if (count >= state->settings.winLineLength - 1 && emptyCount == 1) {
                        // Ïðîâåðÿåì, ÷òî êëåòêà â ïðåäåëàõ ïîëÿ
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

    // 2. Ïðîâåðèòü, íóæíî ëè áëîêèðîâàòü âûèãðûøíûé õîä èãðîêà
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Èùåì êðåñòèêè èãðîêà
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí êðåñòèê
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
                                    found = -1; // Íîëèê ìåøàåò
                                }
                                found = 1;
                                break;
                            }
                        }

                        if (found == 0) { // Ïóñòàÿ êëåòêà
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;

                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) {
                            break; // Íîëèê íà ïóòè
                        }
                    }

                    // Åñëè íóæíî áëîêèðîâàòü (ëèíèÿ èç winLength-1 êðåñòèêîâ ñ îäíîé ïóñòîé)
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
    // 4. Çàíÿòü öåíòð, åñëè îí ñâîáîäåí (òîëüêî äëÿ îãðàíè÷åííîãî ïîëÿ)
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

    // 3. Ïîèñê ñòðàòåãè÷åñêè âàæíûõ ïîçèöèé (äëèííûå ëèíèè)
    // Ñíà÷àëà èùåì ñâîè ëèíèè, êîòîðûå ìîæíî ïðîäîëæèòü
    int bestScore = -1;
    int bestX = 0, bestY = 0;

    // Ïðîâåðÿåì âñå âîçìîæíûå ïóñòûå êëåòêè
    for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
        for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
            // Äëÿ îãðàíè÷åííîãî ïîëÿ ïðîïóñêàåì êëåòêè çà ãðàíèöàìè
            if (state->settings.fieldSize > 0 &&
                (abs(x) > state->settings.fieldSize / 2 ||
                    abs(y) > state->settings.fieldSize / 2)) {
                continue;
            }

            // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà
            int cellFree = 1;
            for (int i = 0; i < state->grid.size; i++) {
                if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                    cellFree = 0;
                    break;
                }
            }

            if (!cellFree) continue;

            // Îöåíèâàåì êëåòêó ïî âñåì íàïðàâëåíèÿì
            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };
            int cellScore = 0;

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                int myCount = 0;   // Íîëèêè
                int oppCount = 0;   // Êðåñòèêè
                int emptyCount = 0;  // Ïóñòûå

                for (int step = -state->settings.winLineLength + 1;
                    step < state->settings.winLineLength; step++) {
                    if (step == 0) continue; // Ïðîïóñêàåì öåíòðàëüíóþ êëåòêó (îíà ïóñòàÿ)

                    int currentX = x + dx * step;
                    int currentY = y + dy * step;

                    // Äëÿ îãðàíè÷åííîãî ïîëÿ ïðîâåðÿåì ãðàíèöû
                    if (state->settings.fieldSize > 0 &&
                        (abs(currentX) > state->settings.fieldSize / 2 ||
                            abs(currentY) > state->settings.fieldSize / 2)) {
                        continue;
                    }

                    // Ïðîâåðÿåì ñîäåðæèìîå êëåòêè
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

                // Îöåíèâàåì ïîòåíöèàë ëèíèè
                if (myCount > 0 && oppCount == 0) {
                    // Ëèíèÿ ñ íàøèìè ñèìâîëàìè - õîðîøàÿ âîçìîæíîñòü
                    cellScore += myCount * myCount;
                }
                else if (oppCount > 0 && myCount == 0) {
                    // Ëèíèÿ ñ ñèìâîëàìè ïðîòèâíèêà - íóæíî áëîêèðîâàòü
                    cellScore += oppCount * oppCount;
                }
            }

            // Îáíîâëÿåì ëó÷øóþ êëåòêó
            if (cellScore > bestScore) {
                bestScore = cellScore;
                bestX = x;
                bestY = y;
            }
        }
    }

    // Åñëè íàøëè õîðîøóþ ïîçèöèþ, äåëàåì õîä
    if (bestScore > 0) {
        addCell(&state->grid, bestX, bestY);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, bestX, bestY, MOVE_AI);
        return;
    }

    

    // 5. Åñëè íè÷åãî ñòðàòåãè÷åñêîãî íå íàéäåíî, äåëàåì õîä ðÿäîì ñ ñóùåñòâóþùèìè íîëèêàìè
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Íàøëè íîëèê
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            // Ïðîâåðÿåì âñå ñîñåäíèå êëåòêè
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;

                    int newX = x + dx;
                    int newY = y + dy;

                    // Ïðîâåðÿåì ãðàíèöû äëÿ îãðàíè÷åííîãî ïîëÿ
                    if (state->settings.fieldSize > 0 &&
                        (abs(newX) > state->settings.fieldSize / 2 ||
                            abs(newY) > state->settings.fieldSize / 2)) {
                        continue;
                    }

                    // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà
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

    // 6. Åñëè âñå îñòàëüíîå íå ñðàáîòàëî, äåëàåì ñëó÷àéíûé õîä
    makeAIMoveMedium(state);
}


// Âñïîìîãàòåëüíàÿ ôóíêöèÿ äëÿ îöåíêè ïîçèöèè (äëÿ ìèíèìàêñà)
int evaluatePosition(AppState* state, int symbol, int opponentSymbol) {
    int score = 0;
    

    // Ïðîâåðÿåì âñå êëåòêè ñ íàøèì ñèìâîëîì
    for (int i = 0; i < state->grid.size; i++) {
        if (state->grid.cells[i].symbol != symbol) continue;

        int x = state->grid.cells[i].x;
        int y = state->grid.cells[i].y;

        int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

        for (int d = 0; d < 4; d++) {
            int dx = directions[d][0];
            int dy = directions[d][1];

            // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
            int count = 1; // Óæå åñòü îäèí íàø ñèìâîë
            int openEnds = 0;
            int blockedEnds = 0;

            // Ïðîâåðÿåì â îäíîì íàïðàâëåíèè
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
                    // Ïðîâåðÿåì ãðàíèöû äëÿ îãðàíè÷åííîãî ïîëÿ
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

            // Ïðîâåðÿåì â ïðîòèâîïîëîæíîì íàïðàâëåíèè
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
                    // Ïðîâåðÿåì ãðàíèöû äëÿ îãðàíè÷åííîãî ïîëÿ
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

            // Îöåíèâàåì ïîòåíöèàëüíóþ ëèíèþ
            if (count >= state->settings.winLineLength) {
                return (symbol == 2) ? 10000 : -10000; // Ïîáåäà/ïîðàæåíèå
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

// Ìèíèìàêñ ñ àëüôà-áåòà îòñå÷åíèåì
int minimax(AppState* state, int depth, int isMaximizing, int alpha, int beta, int* bestX, int* bestY, int aiSymbol, int playerSymbol) {
    // Ïðîâåðÿåì òåðìèíàëüíûå ñîñòîÿíèÿ
    int playerWin = checkWinCondition(state, playerSymbol, state->settings.winLineLength);
    int aiWin = checkWinCondition(state, aiSymbol, state->settings.winLineLength);
    int draw = checkForDraw(state);

    if (aiWin) return 100000 - depth; // ×åì ãëóáæå, òåì ìåíüøå î÷êè
    if (playerWin) return -100000 + depth;
    if (draw) return 0;
    if (depth == 0) return evaluatePosition(state, playerSymbol, aiSymbol);

    if (isMaximizing) {
        int maxEval = -1000000;

        // Ãåíåðèðóåì âîçìîæíûå õîäû
        for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
            for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
                // Ïðîâåðÿåì ãðàíèöû äëÿ îãðàíè÷åííîãî ïîëÿ
                if (state->settings.fieldSize > 0 &&
                    (abs(x) > state->settings.fieldSize / 2 ||
                        abs(y) > state->settings.fieldSize / 2)) {
                    continue;
                }

                // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Äåëàåì õîä
                    addCell(&state->grid, x, y);
                    state->grid.cells[state->grid.size - 1].symbol = aiSymbol;

                    int eval = minimax(state, depth - 1, 0, alpha, beta, NULL, NULL, aiSymbol, playerSymbol);

                    // Îòìåíÿåì õîä
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

        // Ãåíåðèðóåì âîçìîæíûå õîäû
        for (int x = -state->settings.winLineLength; x <= state->settings.winLineLength; x++) {
            for (int y = -state->settings.winLineLength; y <= state->settings.winLineLength; y++) {
                // Ïðîâåðÿåì ãðàíèöû äëÿ îãðàíè÷åííîãî ïîëÿ
                if (state->settings.fieldSize > 0 &&
                    (abs(x) > state->settings.fieldSize / 2 ||
                        abs(y) > state->settings.fieldSize / 2)) {
                    continue;
                }

                // Ïðîâåðÿåì, ÷òî êëåòêà ñâîáîäíà
                int cellFree = 1;
                for (int i = 0; i < state->grid.size; i++) {
                    if (state->grid.cells[i].x == x && state->grid.cells[i].y == y) {
                        cellFree = 0;
                        break;
                    }
                }

                if (cellFree) {
                    // Äåëàåì õîä
                    addCell(&state->grid, x, y);
                    state->grid.cells[state->grid.size - 1].symbol = playerSymbol;

                    int eval = minimax(state, depth - 1, 1, alpha, beta, NULL, NULL, aiSymbol, playerSymbol);

                    // Îòìåíÿåì õîä
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


// Îñíîâíàÿ ôóíêöèÿ äëÿ ýêñïåðòíîãî óðîâíÿ, ðåàëèçàöèÿ ÷àñòè÷íî ÷åðåç ýâðèñòèêè è ÷åðåç ìèíèìàêñà ñ àëüôà áåòà îòñå÷åíèåì, äîâîëüíî õîðîø, íî íå ýôôåêòèâåí íà áîëüøèõ ëèíèÿõ è ïîëÿõ
void makeAIMoveExpert(AppState* state) {
    int aiSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 2 : 1;
    int playerSymbol = (state->settings.firstMove == FIRST_MOVE_PLAYER) ? 1 : 2;

    if (state->grid.size == 0) {
        addCell(&state->grid, 0, 0);
        state->grid.cells[state->grid.size - 1].symbol = aiSymbol;
        logMove(&state->logger, 0, 0, MOVE_AI);
        return;
    }
    // 1. Ïðîâåðèòü, åñòü ëè âûèãðûøíûé õîä äëÿ áîòà (íîëèêà)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == aiSymbol) { // Èùåì ñâîè íîëèêè
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí íîëèê
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

                        if (found == 0) { // Ïóñòàÿ êëåòêà
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;
                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) break;
                    }

                    // Âûèãðûøíûé õîä (íå õâàòàåò îäíîé ôèãóðû)
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

    // 2. Ïðîâåðèòü, íóæíî ëè áëîêèðîâàòü âûèãðûøíûé õîä èãðîêà (íå õâàòàåò îäíîé ôèãóðû)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Èùåì êðåñòèêè èãðîêà
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí êðåñòèê
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

                        if (found == 0) { // Ïóñòàÿ êëåòêà
                            emptyCount++;
                            emptyX = currentX;
                            emptyY = currentY;
                            if (emptyCount > 1) break;
                        }
                        else if (found == -1) break;
                    }

                    // Ñðî÷íàÿ áëîêèðîâêà (íå õâàòàåò îäíîé ôèãóðû)
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

    // 3. Ïðîâåðèòü îïàñíûå ñèòóàöèè (íå õâàòàåò äâóõ ôèãóð)
    for (int i = 0; i < state->grid.size && state->settings.fieldSize != 3; i++) {
        if (state->grid.cells[i].symbol == playerSymbol) { // Èùåì êðåñòèêè èãðîêà
            int x = state->grid.cells[i].x;
            int y = state->grid.cells[i].y;

            int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Ïðîâåðÿåì ëèíèþ â îáîèõ íàïðàâëåíèÿõ
                for (int dir = -1; dir <= 1; dir += 2) {
                    int count = 1; // Óæå åñòü îäèí êðåñòèê
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

                        if (found == 0) { // Ïóñòàÿ êëåòêà
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

                    // Îïàñíàÿ ñèòóàöèÿ (íå õâàòàåò äâóõ ôèãóð)
                    if (count >= state->settings.winLineLength - 2 && emptyCount >= 2) {
                        // Ïðåäïî÷èòàåì áëîêèðîâàòü êëåòêó, êîòîðàÿ ñîçäàñò áîëüøå âîçìîæíîñòåé äëÿ íàñ
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

    // 4. Èñïîëüçîâàòü ìèíèìàêñ äëÿ ñòðàòåãè÷åñêèõ õîäîâ
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

    // 5. Åñëè âñå îñòàëüíîå íå ñðàáîòàëî, èñïîëüçóåì ñòðàòåãèþ èç ñëîæíîãî óðîâíÿ
    makeAIMoveHard(state);
}



int main() {
    
    
    
    // Èíèöèàëèçàöèÿ GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Îøèáêà èíèöèàëèçàöèè GLFW\n");
        return -1;
    }

    // Ñîçäàåì îêíî
    GLFWwindow* window = glfwCreateWindow(1240, 1240, "Krestiki-Noliki", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Îøèáêà ñîçäàíèÿ îêíà\n");
        glfwTerminate();
        return -1;
    }

    // Äåëàåì êîíòåêñò òåêóùèì
    glfwMakeContextCurrent(window);

    // Èíèöèàëèçàöèÿ ñîñòîÿíèÿ ïðèëîæåíèÿ
    AppState state;
    initAppState(&state);
    

    // Èíèöèàëèçàöèÿ òåêñòà
    if (!initText(&state, "text/arial.ttf")) {
        fprintf(stderr, "Íå óäàëîñü çàãðóçèòü øðèôò\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowUserPointer(window, &state);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Îñíîâíîé öèêë ðåíäåðèíãà
    while (!glfwWindowShouldClose(window)) {
        // Î÷èñòêà áóôåðà
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Ïîëó÷àåì ðàçìåðû îêíà
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (state.saveNotificationTimer > 0.0f) {
            state.saveNotificationTimer -= 0.005f; // Óìåíüøàåì íà âðåìÿ êàäðà 
        }
        


        // Âûáèðàåì ÷òî îòðèñîâûâàòü â çàâèñèìîñòè îò ñîñòîÿíèÿ
        switch (state.currentState) {
        case MENU_MAIN:
            drawMainMenu(&state);
            break;

        case MENU_GAME: {
            // Óñòàíàâëèâàåì ìàòðèöó ïðîåêöèè
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            // Ðàññ÷èòûâàåì âèäèìóþ îáëàñòü
            float aspectRatio = (float)width / (float)height;
            float visibleLeft = -1.0f * state.camera.zoom * aspectRatio + state.camera.offsetX;
            float visibleRight = 1.0f * state.camera.zoom * aspectRatio + state.camera.offsetX;
            float visibleBottom = -1.0f * state.camera.zoom + state.camera.offsetY;
            float visibleTop = 1.0f * state.camera.zoom + state.camera.offsetY;

            glOrtho(visibleLeft, visibleRight, visibleBottom, visibleTop, -1.0, 1.0);

            // Óñòàíàâëèâàåì ìàòðèöó ìîäåëè
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glColor3f(0.0, 0.0, 0.0);

            // Îòðèñîâûâàåì ïîëå
            drawGrid(visibleLeft, visibleRight, visibleBottom, visibleTop, state.camera.zoom, state.settings.fieldSize);

            // Îòðèñîâûâàåì âûáðàííóþ êëåòêó
            float cellSize = 2.0f / (10.0f * state.camera.zoom);
            drawSelectedCell(state.selectedCellX, state.selectedCellY, cellSize);

            // Îòðèñîâûâàåì ñèìâîëû
            for (int i = 0; i < state.grid.size; i++) {
                float x1 = state.grid.cells[i].x * cellSize;
                float y1 = state.grid.cells[i].y * cellSize;
                float x2 = x1 + cellSize;
                float y2 = y1 + cellSize;
                
                if (state.grid.cells[i].symbol == 1) { // Êðåñòèê 
                    drawCross(x1, y1, x2, y2);
                }
                else if (state.grid.cells[i].symbol == 2) { // Íîëèê
                    drawCircle(x1, y1, x2, y2);
                }
            }
            if (state.currentState == MENU_SETTINGS) {
                drawSettingsScreen(&state, window);
        }
            if (state.saveNotificationTimer > 0.0f) { // òàéìåð óâåäû î ñåéâå
                drawSaveNotification(&state, width, height);
            }
            if (state.gameResult.rawResult  != 0) { // ïîáåäíàÿ ëèíèÿ
                drawWinLine(&state, width, height);
                if (state.gameResult.isDraw != 1) {
                    drawWinningLine(&state);
                }
            }

            // Ðèñóåì ïîäñêàçêó î ïîìîùè (òîëüêî åñëè ñàìà ïîìîùü íå îòîáðàæàåòñÿ)
            if (!state.showHelp) {
                drawHelpHint(&state, width, height);
            }

            if (state.showMoveLog) {
                drawMoveLog(&state);
            }

            // Åñëè íóæíî ïîêàçàòü ïîìîùü, ðèñóåì ïîâåðõ âñåãî
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

        // Ìåíÿåì áóôåðû è îáðàáàòûâàåì ñîáûòèÿ
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Îñâîáîæäàåì ðåñóðñû
    glDeleteTextures(1, &state.fontTexture);
    cleanupGrid(&state.grid);
    cleanupMoveLogger(&state.logger);
    glfwTerminate();
    
    return 0;
}
