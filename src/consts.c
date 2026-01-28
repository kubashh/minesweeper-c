#include "cross_util.c"

// Colors
// #define RESET               "\033[0m"
#define RESETLN             "\033[0m\n"
#define COLOR_BLACK         "\033[30m"
#define COLOR_RED           "\033[31m"
#define COLOR_GREEN         "\033[32m"
#define COLOR_YELLOW        "\033[33m"
#define COLOR_BLUE          "\033[34m"
#define COLOR_DARKVIOLET    "\033[35m"
#define COLOR_WHITE         "\033[37m"
#define BRIGHT_GREEN        "\033[92m"
#define COLOR_DARKRED       "\033[38;5;124m"
#define COLOR_TOMATO        "\033[38;5;196m"
#define COLOR_STEELBLUE     "\033[38;5;67m"
// #define COLOR_ZINC_900      "\033[38;2;9;9;11m"
#define COLOR_ZINC_950      "\033[38;2;24;24;27m"
#define COLOR_PURPLE_950    "\033[38;2;46;16;101m"

#define COLOR_SELECTED      (COLOR_PURPLE_950 "0" COLOR_WHITE)
#define COLOR_MINE_VALUE    (COLOR_ZINC_950 "M" COLOR_WHITE)

#define BG_BLACK            "\033[48;2;0;0;0m"

// Messages
#define STARTING_MESSAGE (BG_BLACK "Witaj w grze saper! Podczas gry klawisz 'f' wlacza tryb zaznaczania min. Milej zabawy!" RESETLN)
#define SELECT_LEVEL_MESSAGE \
    (BG_BLACK "Podaj poziom trudnosci (" COLOR_GREEN "e - easy" COLOR_WHITE ", " COLOR_YELLOW "m - medium" COLOR_WHITE \
    ", " COLOR_RED "h - hard" COLOR_WHITE ", " COLOR_PURPLE_950 "q - wyjscie" COLOR_WHITE "): %c" RESETLN)
#define MSG_WIN (BG_BLACK "Koniec gry, " BRIGHT_GREEN "wygrales" COLOR_WHITE "! Czas: ")
#define MSG_LOSE (BG_BLACK "Koniec gry, " COLOR_RED "przegrales" COLOR_WHITE "! Czas: ")


// Game
enum CELL: u8 { CELL_MINE = 'm', CELL_FLAG = 'M', CELL_EMPTY = 0 };
enum LEVEL: u8 { EASY = 0, MEDIUM = 1, HARD = 2 };

typedef struct {
    u32 frameDelayMs;
} SaperConfig;

typedef struct {
    u8 x;
    u8 y;
} Vector;

typedef struct {
    u8 value;
    bool isOpened;
    bool isFlagged;
} Cell;

typedef struct {
    u8 minesLeft;
    u64 timeStart;
    i32 lastChar;
    u32 lastTimePlaying;
    u8 winMsg;
    bool isPlaying;
    bool isFirstTime;
    bool isHightlighting;
    bool flagMode;
    Vector targetCell;
    enum LEVEL selectedLevel;
} Game;

typedef struct {
    u8 totalMines;
    u8 x;
    u8 y;
} Level;

const u8 targets[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
SaperConfig saperConfig = {
    .frameDelayMs = 250
};
Game game;

const Level levels[3] = {
    { // Easy
        .totalMines = 10,
        .x = 9,
        .y = 9
    },
    { // Medium
        .totalMines = 40,
        .x = 16,
        .y = 16
    },
    { // Hard
        .totalMines = 99,
        .x = 16,
        .y = 30
    }
};

Level level = levels[EASY];
Cell board[16][30]; // for level hard
const Cell emptyCell = { .value = CELL_EMPTY };

u8 screenBuf[16 * 60 * 10]; // X * Y * colors (+-) big for colors
u32 screenBufLen = 0;
