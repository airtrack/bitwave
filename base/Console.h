#ifndef CONSOLE_H
#define CONSOLE_H

#include "BaseTypes.h"
#include <Windows.h>

class Console : private NotCopyable
{
public:
    Console()
    {
        console_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
    }

    void SetCursorPos(int x, int y)
    {
        COORD cursor;
        cursor.X = x;
        cursor.Y = y;
        ::SetConsoleCursorPosition(console_, cursor);
    }

    void GetCursorPos(int& x, int& y)
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        ::GetConsoleScreenBufferInfo(console_, &info);
        x = info.dwCursorPosition.X;
        y = info.dwCursorPosition.Y;
    }

    void Write(const wchar_t *str, std::size_t size)
    {
        DWORD writed = 0;
        ::WriteConsole(console_, str, size, &writed, 0);
    }

private:
    HANDLE console_;
};

#endif // CONSOLE_H
