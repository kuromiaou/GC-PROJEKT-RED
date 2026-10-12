add_executable(App
    main.cpp
)

target_link_libraries(App
    PRIVATE Renderer
    PRIVATE Platform

)