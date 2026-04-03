cmake_minimum_required(VERSION 3.16)
project(EcoCity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMONGOCXX REQUIRED IMPORTED_TARGET libmongocxx)
pkg_check_modules(LIBBSONCXX REQUIRED IMPORTED_TARGET libbsoncxx)

add_executable(ecocity
    game-engine/main.cpp
    game-engine/domain/City.cpp
    game-engine/domain/Petition.cpp
    game-engine/domain/Resource.cpp
    game-engine/services/GameService.cpp
    game-engine/services/PetitionManager.cpp
    game-engine/services/ResourceManager.cpp
    game-engine/persistence/MongoGameRepository.cpp
)

target_include_directories(ecocity PRIVATE game-engine)
target_link_libraries(ecocity PRIVATE PkgConfig::LIBMONGOCXX PkgConfig::LIBBSONCXX)
