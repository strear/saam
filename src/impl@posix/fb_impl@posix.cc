#include "../saam.hpp"

#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using namespace Saam;

namespace {
    winsize size;
	bool boundDataUpdated = false;
    
	void updateBoundData() {
		if (!boundDataUpdated) {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        }

		boundDataUpdated = !boundDataUpdated;
    }
}

size_t TextFramebuffer::getViewportWidth() {
	updateBoundData();
	return size.ws_col;
}

size_t TextFramebuffer::getViewportHeight() {
	updateBoundData();
	return size.ws_row;
}

bool TextFramebuffer::isMonochrome() {
	return !isatty(STDOUT_FILENO);
}

void TextFramebuffer::_putframe(const char* str, size_t len) {
    static char* buf = new char[128];
    static size_t bufsize = 128;

    if (len + 1 > bufsize) {
        delete[] buf;
        buf = new char[bufsize = len + 1];
    }
    
    memcpy(buf, str, len);
    buf[len] = '\0';

    printf("\033[0;0H%s", buf);
    fflush_unlocked(stdout);
}