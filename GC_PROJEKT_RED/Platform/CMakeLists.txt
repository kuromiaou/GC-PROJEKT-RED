# Platform library
add_library(Platform STATIC)

target_sources(Platform
    PRIVATE
    src/Window.cpp
    src/Mesh.cpp
    src/Texture.cpp
)

target_include_directories(Platform
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/MathsLib
)

# stb_image.h (single-header PNG decoder used by Texture.cpp).
set(STB_DIR ${CMAKE_SOURCE_DIR}/third_party/stb)
if(NOT EXISTS ${STB_DIR}/stb_image.h)
    file(DOWNLOAD
        https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
        ${STB_DIR}/stb_image.h
        TLS_VERIFY ON)
endif()
target_include_directories(Platform PRIVATE ${STB_DIR})

target_compile_definitions(Platform PRIVATE
    WARP_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_compile_features(Platform PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(Platform PRIVATE /W4 /permissive-)
else()
    target_compile_options(Platform PRIVATE -msse4.1)
endif()

target_link_libraries(Platform PRIVATE SDL3::SDL3)