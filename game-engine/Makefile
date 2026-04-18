# Cross-platform Makefile
# Windows: MSVC (cl) + vcpkg  |  Linux: g++ + pkg-config

TARGET := eco_city_engine

PROTO_SRC_DIR := ../game_api/v1
PROTO_FILE    := $(PROTO_SRC_DIR)/api_types.proto
PROTO_GEN_CC  := api_types.pb.cc
PROTO_GEN_H   := api_types.pb.h

# Test sources (Linux only; add MSVC equivalents below if needed)
TEST_SRCS := tests/GameServiceTest.cpp \
             tests/PetitionManagerTest.cpp \
             tests/ResourceManagerTest.cpp \
             tests/test_main.cpp

TEST_TARGET := run_tests

# Integration test sources — each binary gets its own test file + shared main
INTEGRATION_MONGO_SRCS  := tests/IntegrationMongoTest.cpp  tests/test_main.cpp
INTEGRATION_SOCKET_SRCS := tests/IntegrationSocketTest.cpp tests/test_main.cpp

INTEGRATION_MONGO_TARGET  := run_integration_mongo
INTEGRATION_SOCKET_TARGET := run_integration_socket

# Main sources
SRCS := main.cpp \
        domain/Building.cpp \
        domain/buildings/BuildingFactory.cpp \
        domain/City.cpp \
        domain/Petition.cpp \
        domain/Resource.cpp \
        services/GameService.cpp \
        services/PetitionManager.cpp \
        services/ResourceManager.cpp \
        persistence/MongoGameRepository.cpp \
        network/SocketServer.cpp

BUILDING_SRCS := domain/buildings/AirportExpansion.cpp \
                 domain/buildings/HydroelectricPlant.cpp \
                 domain/buildings/IndustrialZone.cpp \
                 domain/buildings/PowerPlant.cpp \
                 domain/buildings/PublicTransportUpgrade.cpp \
                 domain/buildings/RoadImprovement.cpp \
                 domain/buildings/RooftopSolar.cpp \
                 domain/buildings/SolarPanelFarm.cpp \
                 domain/buildings/UrbanGreening.cpp \
                 domain/buildings/WaterSavingInfrastructure.cpp \
                 domain/buildings/WaterTreatmentPlant.cpp \
                 domain/buildings/WindTurbineFarm.cpp

# Platform detection
ifeq ($(OS),Windows_NT)
    # Windows (MSVC + vcpkg)
    CXX        := cl
    TARGET     := $(TARGET).exe
    TEST_TARGET := $(TEST_TARGET).exe
    OBJ_EXT    := .obj
    DEL        := del /q
    FIXPATH     = $(subst /,\\,$1)

    PROTOC := C:/vcpkg/installed/x64-mingw-dynamic/tools/protobuf/protoc.exe

    VCPKG_ROOT := C:/vcpkg/installed/x64-windows
    VCPKG_LIB  := $(VCPKG_ROOT)/lib

    CXXFLAGS := /std:c++17 /W3 /EHsc \
                /I$(VCPKG_ROOT)/include \
                /I$(VCPKG_ROOT)/include/mongocxx/v_noabi \
                /I$(VCPKG_ROOT)/include/bsoncxx/v_noabi

    LDFLAGS := /LIBPATH:$(VCPKG_LIB) /MACHINE:X64
    LIBS    := libprotobuf.lib abseil_dll.lib \
               mongocxx-v_noabi-rhi-md.lib \
               bsoncxx-v_noabi-rhi-md.lib

    # test_main.cpp provides main(), so only gtest.lib is needed here
    TEST_LIBS := $(LIBS) gtest.lib

else
    # Linux (g++ + pkg-config)
    CXX      := g++
    OBJ_EXT  := .o
    DEL      := rm -f
    FIXPATH   = $1

    PROTOC := protoc

    CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic \
                $(shell pkg-config --cflags libmongocxx) \
                $(shell pkg-config --cflags libbsoncxx)

    LDFLAGS  :=
    LIBS     := $(shell pkg-config --libs protobuf) \
                $(shell pkg-config --libs libmongocxx) \
                $(shell pkg-config --libs libbsoncxx)

    # Test libs — assumes Google Test installed via pkg-config or system package
    TEST_LIBS := $(LIBS) $(shell pkg-config --libs gtest gtest_main 2>/dev/null || echo "-lgtest -lgtest_main -lpthread")
endif

# Derived object lists
PROTO_OBJ  := api_types.pb$(OBJ_EXT)

OBJS       := $(SRCS:.cpp=$(OBJ_EXT))
OBJS       += $(BUILDING_SRCS:.cpp=$(OBJ_EXT))

# Test objects reuse all non-main objects, plus test sources
LIB_OBJS   := $(filter-out main$(OBJ_EXT),$(OBJS))
TEST_OBJS  := $(TEST_SRCS:.cpp=$(OBJ_EXT))

INTEGRATION_MONGO_OBJS  := $(INTEGRATION_MONGO_SRCS:.cpp=$(OBJ_EXT))
INTEGRATION_SOCKET_OBJS := $(INTEGRATION_SOCKET_SRCS:.cpp=$(OBJ_EXT))

# Targets
.PHONY: all tests integration_tests clean re

all: $(TARGET)

tests: $(TEST_TARGET)

integration_tests: $(INTEGRATION_MONGO_TARGET) $(INTEGRATION_SOCKET_TARGET)

# Protobuf codegen
$(PROTO_GEN_CC) $(PROTO_GEN_H): $(PROTO_FILE)
	$(PROTOC) --proto_path=$(PROTO_SRC_DIR) --cpp_out=. $(PROTO_FILE)

# Compile proto object
ifeq ($(OS),Windows_NT)
$(PROTO_OBJ): $(PROTO_GEN_CC) $(PROTO_GEN_H)
	$(CXX) $(CXXFLAGS) /c $(PROTO_GEN_CC) /Fo$(PROTO_OBJ)
else
$(PROTO_OBJ): $(PROTO_GEN_CC) $(PROTO_GEN_H)
	$(CXX) $(CXXFLAGS) -c $(PROTO_GEN_CC) -o $(PROTO_OBJ)
endif

# Link main binary
ifeq ($(OS),Windows_NT)
$(TARGET): $(PROTO_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) /Fe$(TARGET) $(PROTO_OBJ) $(OBJS) /link $(LDFLAGS) $(LIBS)
else
$(TARGET): $(PROTO_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(PROTO_OBJ) $(OBJS) $(LDFLAGS) $(LIBS)
endif

# Link test binary
ifeq ($(OS),Windows_NT)
$(TEST_TARGET): $(PROTO_OBJ) $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) /Fe$(TEST_TARGET) $(PROTO_OBJ) $(LIB_OBJS) $(TEST_OBJS) /link $(LDFLAGS) $(TEST_LIBS)
else
$(TEST_TARGET): $(PROTO_OBJ) $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(PROTO_OBJ) $(LIB_OBJS) $(TEST_OBJS) $(LDFLAGS) $(TEST_LIBS)
endif

# Link integration test binaries
ifeq ($(OS),Windows_NT)
$(INTEGRATION_MONGO_TARGET): $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_MONGO_OBJS)
	$(CXX) $(CXXFLAGS) /Fe$(INTEGRATION_MONGO_TARGET).exe $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_MONGO_OBJS) /link $(LDFLAGS) $(TEST_LIBS)

$(INTEGRATION_SOCKET_TARGET): $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_SOCKET_OBJS)
	$(CXX) $(CXXFLAGS) /Fe$(INTEGRATION_SOCKET_TARGET).exe $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_SOCKET_OBJS) /link $(LDFLAGS) $(TEST_LIBS)
else
$(INTEGRATION_MONGO_TARGET): $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_MONGO_OBJS)
	$(CXX) $(CXXFLAGS) -o $(INTEGRATION_MONGO_TARGET) $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_MONGO_OBJS) $(LDFLAGS) $(TEST_LIBS)

$(INTEGRATION_SOCKET_TARGET): $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_SOCKET_OBJS)
	$(CXX) $(CXXFLAGS) -o $(INTEGRATION_SOCKET_TARGET) $(PROTO_OBJ) $(LIB_OBJS) $(INTEGRATION_SOCKET_OBJS) $(LDFLAGS) $(TEST_LIBS)
endif

# Compile rules
ifeq ($(OS),Windows_NT)
%.obj: %.cpp
	$(CXX) $(CXXFLAGS) /c $< /Fo$@
else
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
endif

# Clean
clean:
	$(DEL) $(call FIXPATH,$(OBJS)) \
	       $(call FIXPATH,$(TEST_OBJS)) \
	       $(call FIXPATH,$(INTEGRATION_MONGO_OBJS)) \
	       $(call FIXPATH,$(INTEGRATION_SOCKET_OBJS)) \
	       $(call FIXPATH,$(PROTO_OBJ)) \
	       $(TARGET) $(TEST_TARGET) \
	       $(INTEGRATION_MONGO_TARGET) $(INTEGRATION_SOCKET_TARGET) \
	       $(PROTO_GEN_CC) $(PROTO_GEN_H)
ifeq ($(OS),Windows_NT)
	-$(DEL) *.pdb *.ilk
endif

re: clean all
