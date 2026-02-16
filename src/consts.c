#include "cross_util.c"
#include "stdatomic.h"

// Colors
// #define RESET               "\x1B[0m"
#define RESETLN             "\x1B[0m\n"     // reset + new line
#define COLOR_BLACK         "\x1B[30m"
#define COLOR_RED           "\x1B[31m"
#define COLOR_GREEN         "\x1B[32m"
#define COLOR_YELLOW        "\x1B[33m"
#define COLOR_BLUE          "\x1B[34m"
#define COLOR_DARKVIOLET    "\x1B[35m"
#define COLOR_WHITE         "\x1B[37m"
#define BRIGHT_GREEN        "\x1B[92m"
#define COLOR_DARKRED       "\x1B[38;5;124m"        // tailwindcss text-darkred
#define COLOR_TOMATO        "\x1B[38;5;196m"        // tailwindcss text-tomato
#define COLOR_STEELBLUE     "\x1B[38;5;67m"         // tailwindcss text-steelblue
// #define COLOR_ZINC_900      "\x1B[38;2;9;9;11m"     // tailwindcss text-zinc-900
#define COLOR_ZINC_950      "\x1B[38;2;24;24;27m"   // tailwindcss text-zinc-950
#define COLOR_PURPLE_950    "\x1B[38;2;46;16;101m"  // tailwindcss text-purple-950

#define COLOR_SELECTED      (COLOR_PURPLE_950 "0" COLOR_WHITE)
#define COLOR_MINE_VALUE    (COLOR_ZINC_950 "M" COLOR_WHITE)

#define BG_BLACK            "\x1B[48;2;0;0;0m"

// Messages
#define STARTING_MESSAGE (BG_BLACK "Welcome in Minesweeper game! In game press 'f' key for making mines. Good luck!" RESETLN)
#define SELECT_LEVEL_MESSAGE \
    (BG_BLACK "Specify the difficulty level (" COLOR_GREEN "e - easy" COLOR_WHITE ", " COLOR_YELLOW "m - medium" COLOR_WHITE \
    ", " COLOR_RED "h - hard" COLOR_WHITE ", " COLOR_PURPLE_950 "q - wyjscie" COLOR_WHITE "): %c" RESETLN)
#define MSG_WIN (BG_BLACK "Game over, " BRIGHT_GREEN "Victory" COLOR_WHITE "! Time: ")
#define MSG_LOSE (BG_BLACK "Game over, " COLOR_RED "Defeat" COLOR_WHITE "! Time: ")

// saper config
#define FRAME_DELAY_MS 250


// Game
enum CELL: char { CELL_MINE = 'm', CELL_FLAG = 'M', CELL_EMPTY = 0 };
enum LEVEL { EASY = 0, MEDIUM = 1, HARD = 2 };

typedef struct {
    int x;
    int y;
} Vector;

typedef struct {
    char value;
    bool is_opened;
    bool is_flagged;
} Cell;

typedef struct {
    int mines_left;
    time_t time_start;
    atomic_int last_char;
    int last_time_playing;
    char* win_msg;
    bool is_playing;
    bool is_firstTime;
    bool is_hightlighting;
    bool flag_mode;
    Vector target_cell;
    int selected_level;
} Game;

typedef struct {
    int total_mines;
    int x;
    int y;
} Level;

const int targets[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
Game game = {};

const Level levels[3] = {
    { // Easy
        .total_mines = 10,
        .x = 9,
        .y = 9
    },
    { // Medium
        .total_mines = 40,
        .x = 16,
        .y = 16
    },
    { // Hard
        .total_mines = 99,
        .x = 16,
        .y = 30
    }
};

Level level = levels[EASY];
Cell board[16][30]; // for level hard
const Cell empty_cell = { .value = CELL_EMPTY };

char screen_buf[16 * 60 * 10]; // X * Y * colors (+-) big for colors
size_t screen_buf_len = 0;
