#include "render.c"

// Update

void restartGame() {
    level = levels[game.selectedLevel];

    game.winMsg = 0;
    game.isPlaying = true;
    game.isFirstTime = true;
    game.flagMode = false;
    game.minesLeft = level.totalMines;
    game.timeStart = now_s();
    game.targetCell.x = level.y >> 1;
    game.targetCell.y = level.x >> 1;

    for(u8 x = 0; x < level.x; x++)
        for(u8 y = 0; y < level.y; y++)
            board[x][y] = emptyCell;

    for(u8 i = 0; i < level.totalMines; i++) {
        const u32 x = rand_max(level.x), y = rand_max(level.y);
        if(board[x][y].value == CELL_EMPTY) {
            board[x][y].value = CELL_MINE;
        } else i--;
    }

    for(u8 j = 0; j < level.y; j++)
        for(u8 i = 0; i < level.x; i++) {
            if(board[i][j].value == CELL_MINE) continue;
            for(u8 k = 0; k < 8; k++) {
                const i8 x = i + targets[k][0], y = j + targets[k][1];
                if(x < 0 || y < 0 || x >= level.x || y > level.y) continue;
                if(board[x][y].value == CELL_MINE) {
                    if(board[i][j].value == CELL_EMPTY) board[i][j].value = '0';
                    board[i][j].value++;
                }
            }
        }

    printBoard();
}

void setLevel() {
    switch (game.lastChar) {
        case 'e':
            game.selectedLevel = EASY;
            restartGame();
            break;
        case 'm':
            game.selectedLevel = MEDIUM;
            restartGame();
            break;
        case 'h':
            game.selectedLevel = HARD;
            restartGame();
            break;
        case 'q':
            cross_clear();
            exit(0);
            break;
        default:
            cross_clear();
            printResultsAndSelectLevelMessage();
    }
}

void openCell(u8 x, u8 y) {
    Cell cell = board[x][y];
    if(cell.isOpened) return;
    if(game.flagMode) {
        if(game.minesLeft == 0 && !cell.isFlagged) return;
        board[x][y].isFlagged = !cell.isFlagged;
        game.minesLeft += cell.isFlagged ? 1 : -1;
        printBoard();
        return;
    } else if(cell.isFlagged) return;
    if(cell.value == CELL_MINE) {
        if(game.isFirstTime) {
            restartGame();
            openCell(x, y);
        } else game.isPlaying = false;
    } else {
        if(game.isFirstTime) game.isFirstTime = false;
        board[x][y].isOpened = true;
        if(cell.value == CELL_EMPTY) {
            for(u8 k = 0; k < 8; k++) {
                i8 i = x + targets[k][0], j = y + targets[k][1];
                if(i < 0 || j < 0 || i >= level.x || j >= level.y) continue;
                if(board[i][j].value == CELL_EMPTY) openCell(i, j);
                else board[i][j].isOpened = true;
            }
        }
    }
}

void checkEndGame() {
    bool isWin = false;
    if(game.isPlaying) {
        u8 cellsLeft = 0;
        for(u8 i = 0; i < level.x; i++)
            for(u8 j = 0; j < level.y; j++)
                if(!board[i][j].isOpened) cellsLeft++;
        if(cellsLeft == level.totalMines) {
            isWin = true;
            game.isPlaying = false;
        } else return;
    }

    game.winMsg = isWin ? 1 : 2;

    setLevel();
}

// Events

static inline void keyboardUpdate() {
    if(game.isPlaying)
        switch(game.lastChar) {
            case 'w':
                if(game.targetCell.y > 0) game.targetCell.y--;
                printBoard();
                break;
            case 'a':
                if(game.targetCell.x > 0) game.targetCell.x--;
                printBoard();
                break;
            case 's':
                if(game.targetCell.y < level.x - 1) game.targetCell.y++;
                printBoard();
                break;
            case 'd':
                if(game.targetCell.x < level.y - 1) game.targetCell.x++;
                printBoard();
                break;
            case 'f':
                game.flagMode = !game.flagMode;
                printBoard();
                break;
            case '\n':
                openCell(game.targetCell.y, game.targetCell.x);
                printBoard();
                checkEndGame();
                break;
        }
    else if(!game.isPlaying) setLevel();
}

void* keyboardListener(void* arg) {
    while (game.lastChar = cross_getchar()) {
        if('A' <= game.lastChar && game.lastChar <= 'Z')
            game.lastChar += 'a' - 'A';
        keyboardUpdate();
    }
    return NULL;
}
