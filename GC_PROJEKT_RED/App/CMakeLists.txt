add_executable(App
    main.cpp
)

target_link_libraries(App
    PRIVATE Platform
    PRIVATE AssetEngine
)

add_executable(EcsTest
    EcsTest.cpp
)

target_link_libraries(EcsTest
    PRIVATE Platform
)