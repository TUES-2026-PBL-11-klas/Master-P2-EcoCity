cmake_minimum_required(VERSION 3.20)
project(EcoCity CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies via vcpkg
find_package(protobuf CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Protobuf code generation
# Finds protoc automatically from the same vcpkg installation
get_target_property(PROTOC_EXE protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOC_EXE)
    get_target_property(PROTOC_EXE protobuf::protoc IMPORTED_LOCATION)
endif()

set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../game_api/v1")
set(PROTO_FILE    "${PROTO_SRC_DIR}/api_types.proto")
set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command(
    OUTPUT
        "${PROTO_OUT_DIR}/api_types.pb.cc"
        "${PROTO_OUT_DIR}/api_types.pb.h"
    COMMAND "${PROTOC_EXE}"
        "--proto_path=${PROTO_SRC_DIR}"
        "--cpp_out=${PROTO_OUT_DIR}"
        "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "Running protoc on api_types.proto"
)

add_library(proto_gen STATIC
    "${PROTO_OUT_DIR}/api_types.pb.cc"
)
target_include_directories(proto_gen PUBLIC "${PROTO_OUT_DIR}")
target_link_libraries(proto_gen PUBLIC protobuf::libprotobuf)

# Main game sources
set(GAME_SRCS
    domain/Building.cpp
    domain/buildings/BuildingFactory.cpp
    domain/buildings/AirportExpansion.cpp
    domain/buildings/HydroelectricPlant.cpp
    domain/buildings/IndustrialZone.cpp
    domain/buildings/PowerPlant.cpp
    domain/buildings/PublicTransportUpgrade.cpp
    domain/buildings/RoadImprovement.cpp
    domain/buildings/RooftopSolar.cpp
    domain/buildings/SolarPanelFarm.cpp
    domain/buildings/UrbanGreening.cpp
    domain/buildings/WaterSavingInfrastructure.cpp
    domain/buildings/WaterTreatmentPlant.cpp
    domain/buildings/WindTurbineFarm.cpp
    domain/City.cpp
    domain/Petition.cpp
    domain/Resource.cpp
    services/GameService.cpp
    services/PetitionManager.cpp
    services/ResourceManager.cpp
    persistence/MongoGameRepository.cpp
    network/SocketServer.cpp
)

# Shared library of all non-main code (reused by tests)
add_library(game_lib STATIC ${GAME_SRCS})
target_include_directories(game_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(game_lib PUBLIC PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(game_lib PUBLIC
    proto_gen
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
)

# Main executable
add_executable(eco_city_engine main.cpp)
target_link_libraries(eco_city_engine PRIVATE game_lib)

# Unit tests
add_executable(run_tests
    tests/GameServiceTest.cpp
    tests/PetitionManagerTest.cpp
    tests/ResourceManagerTest.cpp
    tests/test_main.cpp
)
target_link_libraries(run_tests PRIVATE
    game_lib
    GTest::gtest
)

# Integration tests
add_executable(run_integration_mongo
    tests/IntegrationMongoTest.cpp
    tests/test_main.cpp
)
target_link_libraries(run_integration_mongo PRIVATE
    game_lib
    GTest::gtest
)

add_executable(run_integration_socket
    tests/IntegrationSocketTest.cpp
    tests/test_main.cpp
)
target_link_libraries(run_integration_socket PRIVATE
    game_lib
    GTest::gtest
)

# Copy DLLs next to the exe on Windows
if(WIN32)
    add_custom_command(TARGET eco_city_engine POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            $<TARGET_RUNTIME_DLLS:eco_city_engine>
            $<TARGET_FILE_DIR:eco_city_engine>
        COMMAND_EXPAND_LISTS
    )
endif()
