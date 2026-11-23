# Bibliotheque d'archivage : uniquement des en-tetes, aucune dependance au moteur.
add_library(AssetEngine INTERFACE)
target_include_directories(AssetEngine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(AssetEngine INTERFACE cxx_std_20)
