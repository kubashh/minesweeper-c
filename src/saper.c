#include "core.c"
#include "hjson.c"

void* runRender(void* arg) {
    while(true) {
        if(game.isPlaying) printBoard();
        cross_sleep_ms(saperConfig.frameDelayMs);
    }
}

u8 main() {
    randomize();

    u8* configFile = read_file_alloc("saper_config.json");
    if(configFile != NULL) {
        HJson* configJson = HJson_parse(configFile);
        HJson* frameDelayNode = HJson_ObjectAtKey(configJson, "frameDelayMs");
        if(HJson_IsNumber(frameDelayNode)) {
            saperConfig.frameDelayMs = frameDelayNode->number;
        }
        HJson_free(configJson);
        free(configFile);
    }

    setLevel();

    cross_thread_t idKeyboardListener = cross_thread_create(keyboardListener);
    cross_thread_create(runRender);

    cross_thread_join(idKeyboardListener);

    return 0;
}
