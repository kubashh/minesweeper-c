#include "consts.c"

// all printing here

void bufprint(char* str) {
    screen_buf_len += sprintf(&screen_buf[screen_buf_len], str);
}

void update_buf_time() {
    if(game.is_playing) game.last_time_playing = time(NULL);
    time_t time_dif = game.last_time_playing - game.time_start;
    screen_buf_len += sprintf(&screen_buf[screen_buf_len], "%.2d:%.2d", time_dif / 60, time_dif % 60);
}

void update_buf_header() {
    screen_buf_len += sprintf(
        &screen_buf[screen_buf_len],
        game.selected_level == EASY ? BG_BLACK "M:%.2d    %c   " :
        game.selected_level == MEDIUM ? BG_BLACK "M:%.2d    %c                 " :
        BG_BLACK "M:%.2d    %c                                             ",
        game.mines_left,
        game.flag_mode ? CELL_FLAG : ' '
    );
    update_buf_time();
    bufprint(RESETLN);
}

static inline void update_board_screen() {
    size_t index = strlen(screen_buf);
    for(int x = 0; x < level.x; x++) {
        strcpy(&screen_buf[index], BG_BLACK);
        index += sizeof(BG_BLACK) - 1;
        for(int y = 0; y < level.y; y++) {
            const int j = y << 1; // * 2
            if(game.is_hightlighting && x == game.target_cell.y && y == game.target_cell.x) {
                strcpy(&screen_buf[index], COLOR_SELECTED);
                index += sizeof(COLOR_SELECTED) - 1;
            } else if(board[x][y].is_flagged) {
                strcpy(&screen_buf[index], COLOR_MINE_VALUE);
                index += sizeof(COLOR_MINE_VALUE) - 1;
            } else if(board[x][y].is_opened)
                if(board[x][y].value != CELL_EMPTY) {
                    char* str;
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
                        strcpy(&screen_buf[index], str);
                        index += strlen(str);
                    }
                    screen_buf[index++] = board[x][y].value;
                    if(str) {
                        strcpy(&screen_buf[index], COLOR_WHITE);
                        index += sizeof(COLOR_WHITE) - 1;
                    }
                } else screen_buf[index++] = ' ';
            else screen_buf[index++] = '-';
            screen_buf[index++] = ' ';
        }
        strcpy(&screen_buf[index - 1], RESETLN);
        index += sizeof(RESETLN) - 2;
    }
    screen_buf[index] = '\0';
}

void render() {
    cross_clear();
    printf("%s", screen_buf);
    screen_buf_len = 0;
    screen_buf[0] = '\0';
}

void print_board() {
    update_buf_header();
    update_board_screen();

    // Print
    render();

    game.is_hightlighting = !game.is_hightlighting;
}

void print_results_or_select_level_message() {
    if(game.win_msg != NULL) {
        screen_buf_len += sprintf(
            &screen_buf[screen_buf_len],
            game.win_msg
        );

        update_buf_time();
        screen_buf_len += sprintf(
            &screen_buf[screen_buf_len],
            ". Level: %s" RESETLN,
            game.selected_level == EASY ? COLOR_GREEN "easy" COLOR_WHITE :
            game.selected_level == MEDIUM ? COLOR_YELLOW "medium" COLOR_WHITE : COLOR_RED "hard" COLOR_WHITE
        );
    } else { // Print starting message
        bufprint(STARTING_MESSAGE);
    }
    // screen_buf_len += sprintf(&screen_buf[screen_buf_len], SELECT_LEVEL_MESSAGE, game.last_char != '\n' ? game.last_char : ' ');
    render();
    printf(SELECT_LEVEL_MESSAGE, game.last_char != '\n' ? game.last_char : ' ');
}