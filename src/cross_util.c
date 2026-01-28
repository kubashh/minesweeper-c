#ifndef CROSS_UTIL_H
#define CROSS_UTIL_H

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "time.h"
#include "string.h"

//
// bool
//

#ifndef true

#define bool    _Bool
#define true    1
#define false   0

#endif

//
// int, uint
//

#define u8 unsigned char
#define u32 uint32_t
#define u64 uint64_t
#define i8 char
#define i32 int32_t
#define i64 int64_t

#define f32 float
#define f64 double

//
// getchar
//

#ifdef _WIN32
    #include "conio.h"  // _getch
#else
    #include "termios.h" // termios
    #include "unistd.h" // tcgetattr, tcsetattr, read, tcsetattr
#endif

// Cross-platform function to get a single character
u8 cross_getchar() {
#ifdef _WIN32
    return _getch();  // Reads a single key without waiting for Enter
#else
    struct termios oldt, newt;
    u8 ch;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    read(STDIN_FILENO, &ch, 1);  // Read 1 character

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
#endif
}

//
// sleep
//

void cross_sleep_ms(u32 ms);

#ifdef _WIN32
    #include "windows.h"   // Sleep
#else
    #include "unistd.h"    // usleep
#endif

void cross_sleep_ms(u32 ms) {
#ifdef _WIN32
    Sleep(ms); // ms
#else
    usleep(ms * 1000); // ms / 1000
#endif
}

//
// thread
//

#ifdef _WIN32
    #include "windows.h" // HANDLE, DWORD, CreateThread, WaitForSingleObject, CloseHandle
    typedef HANDLE cross_thread_t;
    typedef DWORD thread_return_t;
#else
    #include "pthread.h" // pthread_t, pthread_create, pthread_join
    typedef pthread_t cross_thread_t;
    typedef void* thread_return_t;
#endif

typedef thread_return_t (*thread_func_t)(void *);

i32 cross_thread_create_basic(cross_thread_t *t, thread_func_t func);
cross_thread_t cross_thread_create(thread_func_t func);
i32 cross_thread_join(cross_thread_t t);


#ifdef _WIN32

static DWORD WINAPI win_thread_func(LPVOID arg) {
    thread_func_t func = ((thread_func_t*)arg)[0];
    void *farg = ((void**)arg)[1];
    free(arg);
    return func(farg);
}

i32 cross_thread_create(cross_thread_t *t, thread_func_t func, void *arg) {
    void **pack = malloc(2 * sizeof(void*));
    pack[0] = func;
    pack[1] = arg;
    *t = CreateThread(NULL, 0, win_thread_func, pack, 0, NULL);
    return (*t == NULL);
}

i32 cross_thread_join(cross_thread_t t) {
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
    return 0;
}

#else // LINUX / POSIX

i32 cross_thread_create_basic(cross_thread_t *t, thread_func_t func) {
    return pthread_create(t, NULL, func, NULL);
}

i32 cross_thread_join(cross_thread_t t) {
    return pthread_join(t, NULL);
}

#endif

cross_thread_t cross_thread_create(thread_func_t func) {
    cross_thread_t t;
    cross_thread_create_basic(&t, func);
    return t;
}

//
// read_file
//

u8* read_file_alloc(const u8* const filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return NULL;

    // Move the file pointer to the end of the file to get the size
    fseek(file, 0, SEEK_END);
    u32 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);  // Move back to the beginning of the file

    // Allocate memory for the file content, including space for the null terminator
    u8 *content = (u8 *)malloc(fileSize + 1);
    if (content == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // Read the file contents into the allocated memory
    u32 bytesRead = fread(content, 1, fileSize, file);
    content[bytesRead] = '\0';  // Null-terminate the string

    // Close the file
    fclose(file);

    return content;
}

//
// clear
//

void cross_clear();

void cross_clear() {
#if _WIN32
    system("cls");
#else
    system("clear");
#endif
}

//
// now_s
//

u64 now_s();

u64 now_s() {
    return time(NULL);
}

//
// randomize
//

void randomize();
u32 rand_max(u32 max);

void randomize() {
    srand(time(NULL));
}

u32 rand_max(u32 max) {
    return rand() % max;
}


// saper only
void cps(void *dest, const void *src) { // because of dest type
    u8 *d = dest;
    const u8 *s = src;
    while (*s != '\0') *d++ = *s++;
}

#endif
