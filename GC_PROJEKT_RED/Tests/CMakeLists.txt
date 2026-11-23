# Tests du coeur : ne lient QUE AssetEngine (preuve de son independance).
add_executable(AssetTests AssetTests.cpp)
target_link_libraries(AssetTests PRIVATE AssetEngine)
add_test(NAME AssetTests COMMAND AssetTests)

# Tests des adaptateurs (types generaux + physique).
add_executable(AdapterTests AdapterTests.cpp)
target_link_libraries(AdapterTests PRIVATE Adapters)
add_test(NAME AdapterTests COMMAND AdapterTests)
