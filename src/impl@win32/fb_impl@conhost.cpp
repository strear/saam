#include "../saam.hpp"
#include <windows.h>

using namespace Saam;

namespace {
	HANDLE hOutput = INVALID_HANDLE_VALUE;

    nullptr_t _Initializing_Function_Leader = (
        //SetConsoleCP(GetACP()),
        IsIconic(GetConsoleWindow()) && ShowWindow(GetConsoleWindow(), SW_RESTORE),
        []() {

            // invisible the cursor
            hOutput = CreateConsoleScreenBuffer(
                GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                NULL,
                CONSOLE_TEXTMODE_BUFFER,
                NULL
            );
            CONSOLE_CURSOR_INFO cci;
            cci.bVisible = 0;
            cci.dwSize = 1;
            SetConsoleCursorInfo(hOutput, &cci);

            // enable the console virtual terminal sequences
            DWORD dwMode = 0;
            GetConsoleMode(hOutput, &dwMode);
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOutput, dwMode);

            SetConsoleActiveScreenBuffer(hOutput);

        }(), nullptr);

    COORD size;
    bool boundDataUpdated = false;

    void updateBoundData() {
        if (!boundDataUpdated) {
            if (IsZoomed(GetConsoleWindow())) {
                size = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
            }
            else {
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

                size = {
                    SHORT(csbi.srWindow.Right - csbi.srWindow.Left + 1),
                    SHORT(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
                };
            }
        }

        boundDataUpdated = !boundDataUpdated;
    }
}

size_t TextFramebuffer::getWidth() {
    updateBoundData();
    return size.X;
}

size_t TextFramebuffer::getHeight() {
    updateBoundData();
    return size.Y;
}

bool TextFramebuffer::isMonochrome() {
    return false;
}

void TextFramebuffer::_putstr(const char* str, size_t len) {
	DWORD dWrite = 0;
	SetConsoleCursorPosition(hOutput, { 0, 0 });
	WriteConsoleA(hOutput, str, (DWORD)(size.X * size.Y), &dWrite, NULL);
}