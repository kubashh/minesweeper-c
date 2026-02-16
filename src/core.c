#include "render.c"

// Update

void restart_game() {
    level = levels[game.selected_level];

    game = (Game){
        .win_msg = NULL,
        .last_char = '\0',
        .is_playing = true,
        .is_firstTime = true,
        .flag_mode = false,
        .mines_left = level.total_mines,
        .time_start = time(NULL),
        .target_cell = { .x = level.y >> 1, .y = level.x >> 1 },
        .selected_level = game.selected_level
    };

    for(int x = 0; x < level.x; x++)
        for(int y = 0; y < level.y; y++)
            board[x][y] = empty_cell;

    for(int i = 0; i < level.total_mines; i++) {
        const int x = rand() % level.x, y = rand() % level.y;
        if(board[x][y].value == CELL_EMPTY) {
            board[x][y].value = CELL_MINE;
        } else i--;
    }

    for(int j = 0; j < level.y; j++)
        for(int i = 0; i < level.x; i++) {
            if(board[i][j].value == CELL_MINE) continue;
            for(int k = 0; k < 8; k++) {
                int x = i + targets[k][0], y = j + targets[k][1];
                if(x < 0 || y < 0 || x >= level.x || y > level.y) continue;
                if(board[x][y].value == CELL_MINE) {
                    if(board[i][j].value == CELL_EMPTY) board[i][j].value = '0';
                    board[i][j].value++;
                }
            }
        }

    print_board();
}

void set_level() {
    switch (game.last_char) {
        case 'e':
            game.selected_level = EASY;
            restart_game();
            break;
        case 'm':
            game.selected_level = MEDIUM;
            restart_game();
            break;
        case 'h':
            game.selected_level = HARD;
            restart_game();
            break;
        case 'q':
            exit(0);
            break;
        default:
            cross_clear();
            print_results_or_select_level_message();
    }
}

void open_cell(int x, int y) {
    Cell cell = board[x][y];
    if(cell.is_opened) return;
    if(game.flag_mode) {
        if(game.mines_left == 0 && !cell.is_flagged) return;
        board[x][y].is_flagged = !cell.is_flagged;
        game.mines_left += cell.is_flagged ? 1 : -1;
        print_board();
        return;
    } else if(cell.is_flagged) return;
    if(cell.value == CELL_MINE) {
        if(game.is_firstTime) {
            restart_game();
            open_cell(x, y);
        } else game.is_playing = false;
    } else {
        if(game.is_firstTime) game.is_firstTime = false;
        board[x][y].is_opened = true;
        if(cell.value == CELL_EMPTY) {
            for(int k = 0; k < 8; k++) {
                int i = x + targets[k][0], j = y + targets[k][1];
                if(i < 0 || j < 0 || i >= level.x || j >= level.y) continue;
                if(board[i][j].value == CELL_EMPTY) open_cell(i, j);
                else board[i][j].is_opened = true;
            }
        }
    }
}

void check_end_game() {
    bool is_win = false;
    if(game.is_playing) {
        int cells_left = 0;
        for(int i = 0; i < level.x; i++)
            for(int j = 0; j < level.y; j++)
                if(!board[i][j].is_opened) cells_left++;
        if(cells_left == level.total_mines) {
            is_win = true;
            game.is_playing = false;
        } else return;
    }

    game.win_msg = is_win ? MSG_WIN : MSG_LOSE;

    set_level();
}

// Events

static inline void keyboard_update() {
    if(game.is_playing)
        switch(game.last_char) {
            case 'w':
                if(game.target_cell.y > 0) game.target_cell.y--;
                print_board();
                break;
            case 'a':
                if(game.target_cell.x > 0) game.target_cell.x--;
                print_board();
                break;
            case 's':
                if(game.target_cell.y < level.x - 1) game.target_cell.y++;
                print_board();
                break;
            case 'd':
                if(game.target_cell.x < level.y - 1) game.target_cell.x++;
                print_board();
                break;
            case 'f':
                game.flag_mode = !game.flag_mode;
                print_board();
                break;
            case '\n':
                open_cell(game.target_cell.y, game.target_cell.x);
                print_board();
                check_end_game();
                break;
        }
    else if(!game.is_playing) set_level();
}

void* keyboard_listener(void* arg) {
    while (game.last_char = cross_getchar()) {
        if('A' <= game.last_char && game.last_char <= 'Z')
            game.last_char += 'a' - 'A';
        keyboard_update();
    }
    return NULL;
}
