#include "core.c"

void* run_render(void* arg) {
    while(true) {
        if(game.is_playing) print_board();
        cross_sleep_ms(FRAME_DELAY_MS);
    }
}

int main() {
    srand(time(NULL));

    set_level();

    cross_thread_t keyboard_listener_id = cross_thread_create(keyboard_listener);
    cross_thread_create(run_render); // no need wait for render, only update matters

    cross_thread_join(keyboard_listener_id);

    return 0;
}
