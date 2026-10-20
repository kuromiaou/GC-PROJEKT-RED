# Platform library
add_library(Platform STATIC)

target_sources(Platform
    PRIVATE
    src/Window.cpp
    src/Mesh.cpp
)

target_include_directories(Platform
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/MathsLib
)
target_compile_definitions(Platform PRIVATE
    WARP_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_compile_features(Platform PUBLIC cxx_std_20)

# Your Vec/Matrix code uses SSE4.1 (_mm_dp_ps, _mm_blend_ps).
if(MSVC)
    target_compile_options(Platform PRIVATE /W4 /permissive-)
else()
    target_compile_options(Platform PRIVATE -msse4.1)
endif()

target_link_libraries(Platform PRIVATE SDL3::SDL3)