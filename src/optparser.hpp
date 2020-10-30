#pragma once

namespace Tuiapp {
    class Cmdline {
    public:
        Cmdline(int argc, char** argv);

        const char* appname();
        bool get(const char* key, char* valueSaver = nullptr);


        


    };
}