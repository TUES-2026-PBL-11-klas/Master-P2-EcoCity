CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
TARGET := eco_city_engine
TEST_NAMES := ResourceManagerTest PetitionManagerTest GameServiceTest
TEST_MAIN := tests/test_main.cpp
TEST_BUILDING_SRCS := $(filter-out domain/buildings/BuildingFactory.cpp,$(wildcard domain/buildings/*.cpp))

PROTOC := C:/msys64/ucrt64/bin/protoc.exe

PROTO_SRC_DIR := ../game_api/v1
PROTO_FILE := $(PROTO_SRC_DIR)/api_types.proto

PROTO_GEN_CC := api_types.pb.cc
PROTO_GEN_H := api_types.pb.h

ifeq ($(OS),Windows_NT)
    PROTO_OBJ := api_types.pb.obj
else
    PROTO_OBJ := api_types.pb.o
endif

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
				domain/buildings/WindTurbineFarm.cpp \

ifeq ($(OS),Windows_NT)
    OBJ_EXT := .obj
    TEST_EXE_EXT := .exe
else
    OBJ_EXT := .o
    TEST_EXE_EXT :=
endif

OBJS := $(SRCS:.cpp=$(OBJ_EXT))

OBJS += $(BUILDING_SRCS:.cpp=$(OBJ_EXT))
TEST_EXES := $(addprefix tests/,$(addsuffix $(TEST_EXE_EXT),$(TEST_NAMES)))

ifeq ($(OS),Windows_NT)
    CXX := cl
    TARGET := $(TARGET).exe
    DEL := del /q
    FIXPATH = $(subst /,\\,$1)
    VCPKG_ROOT := C:/vcpkg/installed/x64-windows
    VCPKG_LIB := $(VCPKG_ROOT)/lib
    CXXFLAGS := /std:c++17 /W3 /EHsc \
                /I$(VCPKG_ROOT)/include \
                /I$(VCPKG_ROOT)/include/mongocxx/v_noabi \
                /I$(VCPKG_ROOT)/include/bsoncxx/v_noabi
    LDFLAGS := /LIBPATH:$(VCPKG_LIB) /MACHINE:X64
    LIBS := libprotobuf.lib abseil_dll.lib mongocxx-v_noabi-rhi-md.lib bsoncxx-v_noabi-rhi-md.lib

    TEST_CXX ?= C:/msys64/ucrt64/bin/g++.exe
    PKG_CONFIG ?= C:/msys64/ucrt64/bin/pkg-config.exe
    GTEST_ROOT ?= C:/msys64/ucrt64
    TEST_CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -I$(GTEST_ROOT)/include
    TEST_LDFLAGS ?= -L$(GTEST_ROOT)/lib
    TEST_LDLIBS ?= -lgtest
    PROTOBUF_ROOT ?= C:/msys64/ucrt64
    PROTOBUF_CXXFLAGS ?= $(shell $(PKG_CONFIG) --cflags protobuf)
    PROTOBUF_LDFLAGS ?=
    PROTOBUF_LDLIBS ?= $(shell $(PKG_CONFIG) --libs protobuf)
else
    DEL := rm -f
    FIXPATH = $1
    LDFLAGS :=
    LIBS := `pkg-config --libs protobuf` \
            `pkg-config --libs libmongocxx` \
            `pkg-config --libs libbsoncxx`
    CXXFLAGS += `pkg-config --cflags libmongocxx` \
                `pkg-config --cflags libbsoncxx`

    TEST_CXX ?= g++
    GTEST_ROOT ?=
    TEST_CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic `pkg-config --cflags gtest`
    TEST_LDFLAGS ?=
    TEST_LDLIBS ?= `pkg-config --libs gtest`
    PROTOBUF_CXXFLAGS ?= `pkg-config --cflags protobuf`
    PROTOBUF_LDFLAGS ?=
    PROTOBUF_LDLIBS ?= `pkg-config --libs protobuf`
endif

all: $(TARGET)
test: $(TEST_EXES)
test-resource: tests/ResourceManagerTest$(TEST_EXE_EXT)
test-petition: tests/PetitionManagerTest$(TEST_EXE_EXT)
test-game-service: tests/GameServiceTest$(TEST_EXE_EXT)

test-help:
	@echo Test commands:
	@echo   mingw32-make test
	@echo   mingw32-make run-tests
	@echo   mingw32-make test-resource
	@echo   mingw32-make test-petition
	@echo   mingw32-make test-game-service
	@echo.
	@echo Google Test setup:
	@echo   Windows default expects GTEST_ROOT=C:/msys64/ucrt64
	@echo   Install package: mingw-w64-ucrt-x86_64-gtest
	@echo   Or override manually, for example:
	@echo   mingw32-make test GTEST_ROOT=C:/path/to/gtest TEST_CXX=C:/path/to/g++.exe

$(PROTO_GEN_CC) $(PROTO_GEN_H): $(PROTO_FILE)
	$(PROTOC) --proto_path=$(PROTO_SRC_DIR) --cpp_out=. $(PROTO_FILE)

ifeq ($(OS),Windows_NT)
$(PROTO_OBJ): $(PROTO_GEN_CC) $(PROTO_GEN_H)
	$(CXX) $(CXXFLAGS) /c $(PROTO_GEN_CC) /Fo$(PROTO_OBJ)
else
$(PROTO_OBJ): $(PROTO_GEN_CC) $(PROTO_GEN_H)
	$(CXX) $(CXXFLAGS) -c $(PROTO_GEN_CC) -o $(PROTO_OBJ)
endif

ifeq ($(OS),Windows_NT)
$(TARGET): $(PROTO_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) /Fe$(TARGET) $(PROTO_OBJ) $(OBJS) /link $(LDFLAGS) $(LIBS)
else
$(TARGET): $(PROTO_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(PROTO_OBJ) $(OBJS) $(LDFLAGS) $(LIBS)
endif

ifeq ($(OS),Windows_NT)
%.obj: %.cpp
	$(CXX) $(CXXFLAGS) /c $< /Fo$@
else
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
endif

tests/ResourceManagerTest$(TEST_EXE_EXT): tests/ResourceManagerTest.cpp $(TEST_MAIN) domain/Resource.cpp services/ResourceManager.cpp
	$(TEST_CXX) $(TEST_CXXFLAGS) $(TEST_MAIN) tests/ResourceManagerTest.cpp domain/Resource.cpp services/ResourceManager.cpp -o $@ $(TEST_LDFLAGS) $(TEST_LDLIBS)

tests/PetitionManagerTest$(TEST_EXE_EXT): tests/PetitionManagerTest.cpp $(TEST_MAIN) domain/Building.cpp domain/Petition.cpp services/PetitionManager.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS)
	$(TEST_CXX) $(TEST_CXXFLAGS) $(TEST_MAIN) tests/PetitionManagerTest.cpp domain/Building.cpp domain/Petition.cpp services/PetitionManager.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS) -o $@ $(TEST_LDFLAGS) $(TEST_LDLIBS)

tests/GameServiceTest$(TEST_EXE_EXT): tests/GameServiceTest.cpp $(TEST_MAIN) api_types.pb.cc domain/Building.cpp domain/City.cpp domain/Petition.cpp domain/Resource.cpp services/ResourceManager.cpp services/PetitionManager.cpp services/GameService.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS)
	$(TEST_CXX) $(TEST_CXXFLAGS) $(PROTOBUF_CXXFLAGS) $(TEST_MAIN) tests/GameServiceTest.cpp api_types.pb.cc domain/Building.cpp domain/City.cpp domain/Petition.cpp domain/Resource.cpp services/ResourceManager.cpp services/PetitionManager.cpp services/GameService.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS) -o $@ $(TEST_LDFLAGS) $(PROTOBUF_LDFLAGS) $(TEST_LDLIBS) $(PROTOBUF_LDLIBS)

run-tests: $(TEST_EXES)
	$(foreach test_exe,$(TEST_EXES),$(test_exe) &&) exit 0

clean:
	$(DEL) $(call FIXPATH,$(OBJS)) $(TARGET) $(PROTO_GEN_CC) $(PROTO_GEN_H) $(PROTO_OBJ)
ifeq ($(OS),Windows_NT)
	-$(DEL) *.pdb *.ilk
endif

clean-tests:
	-$(DEL) $(call FIXPATH,$(TEST_EXES))

re: clean all

.PHONY: all clean clean-tests re test test-resource test-petition test-game-service run-tests test-help
