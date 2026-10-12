# Renderer library
add_library(Renderer STATIC)

# Sources
target_sources(Renderer
    PRIVATE
	src/Renderer.cpp
)

target_include_directories(Renderer
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# C++20
target_compile_features(Renderer PUBLIC cxx_std_20)

# Compiler options MSVC
if(MSVC)
    target_compile_options(Renderer PRIVATE /W4 /permissive-)
endif()

