#include "Platform/Window.h"
#include "AssetEngine/BinaryWriter.h"
#include "AssetEngine/BinaryReader.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

struct Header {
    char magic[4];
    std::uint8_t version;
    std::uint64_t directoryOffset;
};

int main() {
    std::cout << "Size: " << sizeof(Header) << '\n';
    std::cout << "Alignment: " << alignof(Header) << '\n';

    std::cout << "Directory offset: " << offsetof(Header, directoryOffset) << '\n';

    BinaryWriter writer;

    writer.WriteU8('G');
    writer.WriteU8('A');
    writer.WriteU8('S');
    writer.WriteU8('T');

    writer.WriteU32(1);
    writer.WriteU32(42);
    writer.WriteString("Player");

    BinaryReader reader(writer.Bytes());

    std::uint32_t id = 0;
    std::string name;

    if (reader.ReadU32(id) && reader.ReadString(name)) {
        // id == 42
        // name == "Player"
    }
}

//int main()
//{
//    Window window;
//    return window.TestWindow();
//}
