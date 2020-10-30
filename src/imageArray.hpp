#pragma once

#include <cstdint>
#include "Array.hpp"

namespace Saam {
    class PicLoader {
        void* impl;

    public:
        PicLoader();
        ~PicLoader();

        void loadPic(const char* file);
        void getPixels(Array<uint8_t>& receiver);
    };
}