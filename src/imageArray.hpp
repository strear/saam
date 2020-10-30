#pragma once

#include "Array.hpp"
#include <cstdint>

namespace Saam {
    class PicLoader {
        void* impl;

    public:
        PicLoader();
        ~PicLoader();

        void loadPic(const char* file);
        void getPixels(Array<int8_t>& receiver);
    };
}