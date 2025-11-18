#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <stdio.h>

static HANDLE hConsole = NULL;

inline void InitConsole() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    system("cls");
}

inline void SetConsoleCursor(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(hConsole, coord);
}

inline void PrintAt(int x, int y, const char* text, WORD color = 7) {
    SetConsoleCursor(x, y);
    SetConsoleTextAttribute(hConsole, color);
    printf("%s", text);
}

inline void PrintAtf(int x, int y, WORD color, const char* format, ...) {
    SetConsoleCursor(x, y);
    SetConsoleTextAttribute(hConsole, color);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

inline int RandomRange(int min, int max) {
    return min + (rand() % (max - min + 1));
}

inline char RandomKey() {
    return 'A' + (rand() % 26);
}

#define COLOR_WHITE 7
#define COLOR_GREEN 10
#define COLOR_RED 12
#define COLOR_YELLOW 14
#define COLOR_CYAN 11
#define COLOR_MAGENTA 13

#endif // UTILS_H
