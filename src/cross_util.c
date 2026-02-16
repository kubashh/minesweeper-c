#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "time.h"
#include "string.h"

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
int cross_getchar() {
#ifdef _WIN32
    return _getch();  // Reads a single key without waiting for Enter
#else
    struct termios oldt, newt;
    char ch;

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

void cross_sleep_ms(int ms);

#ifdef _WIN32
    #include "windows.h"   // Sleep
#else
    #include "unistd.h"    // usleep
#endif

void cross_sleep_ms(int ms) {
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

int cross_thread_create_basic(cross_thread_t *t, thread_func_t func);
cross_thread_t cross_thread_create(thread_func_t func);
int cross_thread_join(cross_thread_t t);


#ifdef _WIN32

static DWORD WINAPI win_thread_func(LPVOID arg) {
    thread_func_t func = ((thread_func_t*)arg)[0];
    void *farg = ((void**)arg)[1];
    free(arg);
    return func(farg);
}

int cross_thread_create(cross_thread_t *t, thread_func_t func, void *arg) {
    void **pack = malloc(2 * sizeof(void*));
    pack[0] = func;
    pack[1] = arg;
    *t = CreateThread(NULL, 0, win_thread_func, pack, 0, NULL);
    return (*t == NULL);
}

int cross_thread_join(cross_thread_t t) {
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
    return 0;
}

#else // LINUX / POSIX

int cross_thread_create_basic(cross_thread_t *t, thread_func_t func) {
    return pthread_create(t, NULL, func, NULL);
}

int cross_thread_join(cross_thread_t t) {
    return pthread_join(t, NULL);
}

#endif

cross_thread_t cross_thread_create(thread_func_t func) {
    cross_thread_t t;
    cross_thread_create_basic(&t, func);
    return t;
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

void clear_n(int n) {
    char buf[16];
    sprintf(buf, "\x1B[%dD", n);
    printf(buf);
}
