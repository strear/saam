#pragma once

#include <ctime>

namespace Tuiapp {
    class Cmdline {
    public:
        Cmdline(int argc, char** argv);
        ~Cmdline();

        const char* appname();
        bool get(const char* key);
        bool get(const char key);
        bool get(const char* key, const char*& valueSaver);
        bool get(const char key, const char*& valueSaver);

        const char* firstPending();
        int remainc();
    };

    bool parseInt(const char* src, int& dst);
    void quit(int retcode);

    enum FileAccessState : int {
        F_OK, R_OK, W_OK, X_OK
    };

    bool checkFile(const char* path, FileAccessState);

    void sleep(clock_t duration);
}