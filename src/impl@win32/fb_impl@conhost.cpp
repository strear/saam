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

		}(), nullptr);

	bool inited = false;

	COORD size;
	bool boundDataUpdated = false;

	void updateBoundData() {
		if (!boundDataUpdated) {
			if (!inited)
				SetConsoleActiveScreenBuffer(hOutput);

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo(hOutput, &csbi);

			size = {
				SHORT(csbi.srWindow.Right - csbi.srWindow.Left + 1),
				SHORT(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
			};
		}

		boundDataUpdated = !boundDataUpdated;
	}
}

size_t TextFramebuffer::getViewportWidth() {
	updateBoundData();
	return size.X;
}

size_t TextFramebuffer::getViewportHeight() {
	updateBoundData();
	return size.Y;
}

bool TextFramebuffer::isMonochrome() {
	return false;
}

void TextFramebuffer::_putframe(const char* str, size_t len) {
	DWORD dWrite = 0;
	SetConsoleCursorPosition(hOutput, { 0, 0 });
	WriteConsoleA(hOutput, str, (DWORD)len, &dWrite, NULL);
}