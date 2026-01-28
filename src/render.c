#include "consts.c"

// all printing here

static inline void bufprint(char* str) {
    screenBufLen += sprintf(&screenBuf[screenBufLen], str);
}

static inline void updateBufTime() {
    if(game.isPlaying) game.lastTimePlaying = now_s();
    u32 timeDif = game.lastTimePlaying - game.timeStart;
    screenBufLen += sprintf(&screenBuf[screenBufLen], "%.2d:%.2d", timeDif / 60, timeDif % 60);
}

static inline void updateBufHeader() {
    screenBufLen += sprintf(
        &screenBuf[screenBufLen],
        game.selectedLevel == EASY ? BG_BLACK "M:%.2d    %c   " :
        game.selectedLevel == MEDIUM ? BG_BLACK "M:%.2d    %c                 " :
        BG_BLACK "M:%.2d    %c                                             ",
        game.minesLeft,
        game.flagMode ? CELL_FLAG : ' '
    );
    updateBufTime();
    bufprint(RESETLN);
}

static inline void updateBoardScreen() {
    u32 index = strlen(screenBuf);
    for(u8 x = 0; x < level.x; x++) {
        strcpy(&screenBuf[index], BG_BLACK);
        index += sizeof(BG_BLACK) - 1;
        for(u8 y = 0; y < level.y; y++) {
            const u8 j = y << 1; // * 2
            if(game.isHightlighting && x == game.targetCell.y && y == game.targetCell.x) {
                strcpy(&screenBuf[index], COLOR_SELECTED);
                index += sizeof(COLOR_SELECTED) - 1;
            } else if(board[x][y].isFlagged) {
                strcpy(&screenBuf[index], COLOR_MINE_VALUE);
                index += sizeof(COLOR_MINE_VALUE) - 1;
            } else if(board[x][y].isOpened)
                if(board[x][y].value != CELL_EMPTY) {
                    u8* str;
                    switch(board[x][y].value) {
                        case '1':
                            str = COLOR_BLUE;
                            break;
                        case '2':
                            str = COLOR_GREEN;
                            break;
                        case '3':
                            str = COLOR_TOMATO;
                            break;
                        case '4':
                            str = COLOR_DARKVIOLET;
                            break;
                        case '5':
                            str = COLOR_DARKRED;
                            break;
                        case '6':
                            str = COLOR_STEELBLUE;
                            break;
                        case '7':
                        case '8':
                            str = COLOR_BLACK;
                            break;
                    }
                    if(str) {
                        strcpy(&screenBuf[index], str);
                        index += strlen(str);
                    }
                    screenBuf[index++] = board[x][y].value;
                    if(str) {
                        strcpy(&screenBuf[index], COLOR_WHITE);
                        index += sizeof(COLOR_WHITE) - 1;
                    }
                } else screenBuf[index++] = ' ';
            else screenBuf[index++] = '-';
            screenBuf[index++] = ' ';
        }
        strcpy(&screenBuf[index - 1], RESETLN);
        index += sizeof(RESETLN) - 2;
    }
    screenBuf[index] = '\0';
}

static inline void render() {
    cross_clear();
    printf("%s", screenBuf);
    screenBufLen = 0;
    screenBuf[0] = '\0';
}

static inline void printBoard() {
    cross_clear();
    updateBufHeader();
    updateBoardScreen();

    // Print
    render();

    game.isHightlighting = !game.isHightlighting;
}

static inline void printResultsAndSelectLevelMessage() {
    if(game.winMsg > 0) {
        screenBufLen += sprintf(
            &screenBuf[screenBufLen],
            "\033[48;2;0;0;0m" "Koniec gry, %s! Czas: ",
            game.winMsg == 2 ? "\033[31m" "przegrales" "\033[37m" : "\033[92m" "wygrales" "\033[37m"
        );

        updateBufTime();
        screenBufLen += sprintf(
            &screenBuf[screenBufLen],
            ". Poziom: %s" RESETLN,
            game.selectedLevel == EASY ? COLOR_GREEN "easy (latwy)" COLOR_WHITE :
            game.selectedLevel == MEDIUM ? COLOR_YELLOW "medium (sredni)" COLOR_WHITE : COLOR_RED "hard (trudny)" COLOR_WHITE
        );
    } else { // Print starting message
        bufprint(STARTING_MESSAGE);
    }
    // screenBufLen += sprintf(&screenBuf[screenBufLen], SELECT_LEVEL_MESSAGE, game.lastChar != '\n' ? game.lastChar : ' ');
    render();
    printf(SELECT_LEVEL_MESSAGE, game.lastChar != '\n' ? game.lastChar : ' ');
}