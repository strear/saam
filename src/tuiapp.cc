#include "tuiapp.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <list>
#include <thread>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace Tuiapp;

namespace {
	struct opt_pair {
		std::string option;
		char* value = nullptr;
	};

	char singleCharField[2] = {};

	char* _basename(char* path) {
		for (char* cptr = path + strlen(path); cptr != path; cptr--)
			if (*cptr == '/' || *cptr == '\\')
				return cptr + 1;
		return path;
	}
}

#define entities_typed ((std::list<opt_pair>*)entities)

Cmdline::Cmdline(int argc, char** argv)
	: appname_keeper(_basename(argv[0]))
	, entities(new std::list<opt_pair>()) {

	bool checkOptions = true, attachAsValue = false;

	for (int i = 1; i < argc; i++) {
		if (checkOptions && argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (argv[i][2] == '\0') {
					checkOptions = false;
					continue;
				}

				char* cptr = &argv[i][2] + strlen(&argv[i][2]);
				for (; cptr != &argv[i][2]; cptr--) {
					if (*cptr == '=') break;
				}

				if (cptr != &argv[i][2]) {
					entities_typed->push_back(
						{ std::string(&argv[i][2], cptr - &argv[i][2]), cptr + 1 });
					attachAsValue = false;
				} else {
					entities_typed->push_back({ std::string(&argv[i][2]) });
					attachAsValue = true;
				}
			} else {
				for (const char* cptr = &argv[i][1]; *cptr != '\0'; cptr++) {
					entities_typed->push_back({ std::string(1, *cptr) });
					attachAsValue = true;
				}
			}
		} else {
			if (attachAsValue) {
				entities_typed->back().value = argv[i];
			} else {
				entities_typed->push_back({ "", argv[i] });
			}
		}
	}
}

Cmdline::~Cmdline() { delete entities_typed; }

size_t Tuiapp::Cmdline::remainc() const {
	return entities_typed->size();
}

bool Cmdline::get(const char* opt) {
	auto it = std::find_if(entities_typed->begin(), entities_typed->end(),
		[&](opt_pair& a) { return a.option == opt; });
	if (it == entities_typed->end()) return false;

	it->option = "";
	if (it->value == nullptr) entities_typed->erase(it);
	return true;
}

bool Cmdline::get(const char opt) {
	singleCharField[0] = opt;
	return get(singleCharField);
}

bool Cmdline::get(const char* opt, const char*& valueSaver) {
	auto it = std::find_if(entities_typed->begin(), entities_typed->end(),
		[&](opt_pair& a) { return a.option == opt; });
	if (it == entities_typed->end()) return false;

	valueSaver = it->value;
	entities_typed->erase(it);
	return true;
}

bool Cmdline::get(const char opt, const char*& valueSaver) {
	singleCharField[0] = opt;
	return get(singleCharField, valueSaver);
}

std::string Cmdline::firstPendOption() {
	auto it = std::find_if(entities_typed->begin(), entities_typed->end(),
		[](opt_pair& a) { return !a.option.empty(); });

	return it != entities_typed->end() ? it->option : "";
}

const char* Cmdline::popValue() {
	auto it = std::find_if(entities_typed->begin(), entities_typed->end(),
		[](opt_pair& a) { return a.option.empty(); });
	if (it == entities_typed->end()) return nullptr;

	const char* ans = it->value;
	entities_typed->erase(it);
	return ans;
}

bool Tuiapp::accessible(const char* path, FileAccessState state) {
#ifdef _MSC_VER
	if (int(state) & int(FileAccessState::X_OK)) {
		state = FileAccessState(
			(int(state) | int(FileAccessState::R_OK)) & !int(FileAccessState::X_OK));
	}
	return _access_s(path, int(state)) == 0;
#else
	return access(path, int(state)) == 0;
#endif
}

void Tuiapp::sleep(clock_t ms) {
	if (ms <= 0) return;
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}