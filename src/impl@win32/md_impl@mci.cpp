#include "../media.hpp"

#undef UNICODE
#include <windows.h>

#pragma comment(lib, "winmm.lib")

using namespace Saam;

namespace {
	struct ImplData {
		UINT id;
	};
}

Media::Media(const char* file) : impl(new ImplData()) {
	MCI_OPEN_PARMS openp;
	openp.lpstrElementName = file;
	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT, (DWORD_PTR)&openp);
	((ImplData*)impl)->id = openp.wDeviceID;
}

Media::~Media() {
	mciSendCommand(((ImplData*)impl)->id, MCI_STOP, MCI_WAIT, NULL);
	mciSendCommand(((ImplData*)impl)->id, MCI_CLOSE, 0, NULL);
	delete (ImplData*)impl;
}

void Media::play() {
	MCI_PLAY_PARMS mciPlayParms;
	mciSendCommand(((ImplData*)impl)->id, MCI_PLAY, 0,
		(DWORD_PTR)&mciPlayParms);
}

void Media::pause() {
	MCI_GENERIC_PARMS params;
	params.dwCallback = NULL;
	mciSendCommand(((ImplData*)impl)->id, MCI_PAUSE,
		MCI_WAIT, (DWORD_PTR)&params);
}

void Media::reset() {
	MCI_PLAY_PARMS mciPlayParms;
	mciSendCommand(((ImplData*)impl)->id, MCI_SEEK,
		MCI_WAIT | MCI_SEEK_TO_START, (DWORD_PTR)&mciPlayParms);
}