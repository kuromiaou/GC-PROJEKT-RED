# Adaptateurs : types generaux (Player, Inventory) et scene physique.
# Ils utilisent AssetEngine ; AssetEngine ne les connait pas.
add_library(Adapters INTERFACE)
target_include_directories(Adapters INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Adapters INTERFACE AssetEngine)
