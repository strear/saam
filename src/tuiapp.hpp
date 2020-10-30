#pragma once

#include <ctime>
#include <cstdlib>
#include <string>

namespace Tuiapp {
    class Cmdline {
        const char* appname_keeper;
        void* entities;

    public:
        Cmdline(int argc, char** argv);
        ~Cmdline();

        const char* appname() { return appname_keeper; }

        bool get(const char* opt);
        bool get(const char opt);
        bool get(const char* opt, const char*& valueSaver);
        bool get(const char opt, const char*& valueSaver);

        std::string firstPendOption();
        const char* popValue();
        size_t remainc() const;
    };

    enum class FileAccessState : int {
        F_OK = 0, R_OK = 4, W_OK = 2, X_OK = 1
    };

    bool accessible(const char* path, FileAccessState);

    void sleep(clock_t milliseconds);

    template <typename T>
    bool parseInt(const char* src, T& dst) {
        try {
            dst = std::stoi(std::string(src));
            return true;
        } catch (std::exception e) {
            return false;
        }
    }
}