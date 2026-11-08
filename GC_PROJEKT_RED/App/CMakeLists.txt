add_executable(App
    main.cpp
)

target_link_libraries(App
    PRIVATE Platform
)

add_executable(EcsTest
    EcsTest.cpp
)

target_link_libraries(EcsTest
    PRIVATE Platform
)