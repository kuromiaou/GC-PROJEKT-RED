# Platform library
add_library(Platform STATIC)

# Sources
target_sources(Platform
    PRIVATE
	src/Window.cpp
)


target_include_directories(Platform
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/MathsLib
)


# C++20
target_compile_features(Platform PUBLIC cxx_std_20)

# Compiler options MSVC
if(MSVC)
    target_compile_options(Platform PRIVATE /W4 /permissive-)
endif()

# Link libraries
target_link_libraries(Platform
    PRIVATE
	SDL3::SDL3
)