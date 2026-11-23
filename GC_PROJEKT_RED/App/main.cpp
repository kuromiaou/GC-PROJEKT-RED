#include "Platform/Window.h"


#include <cstddef>
#include <cstdint>
#include <iostream>

struct Header {
    char magic[4];
    std::uint8_t version;
    std::uint64_t directoryOffset;
};

int main() {}

//int main()
//{
//    Window window;
//    return window.TestWindow();
//}
