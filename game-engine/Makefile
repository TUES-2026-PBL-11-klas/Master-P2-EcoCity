UNAME_S := $(shell uname -s 2>/dev/null)

CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
PROTO_SRC_DIR := ../game_api/v1
PROTO_FILE := $(PROTO_SRC_DIR)/api_types.proto
PROTO_GEN_CC := api_types.pb.cc
PROTO_GEN_H := api_types.pb.h
TEST_NAMES := ResourceManagerTest PetitionManagerTest GameServiceTest
TEST_MAIN := tests/test_main.cpp
TEST_BUILDING_SRCS := $(filter-out domain/buildings/BuildingFactory.cpp,$(wildcard domain/buildings/*.cpp))

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

ifeq ($(UNAME_S),Linux)

CXX ?= g++
PKG_CONFIG ?= pkg-config
PROTOC ?= protoc
TARGET := eco_city_engine
OBJ_EXT := .o
TEST_EXE_EXT :=
DEL := rm -f
FIXPATH = $1

PROTO_OBJ := api_types.pb.o
OBJS := $(SRCS:.cpp=$(OBJ_EXT))
OBJS += $(BUILDING_SRCS:.cpp=$(OBJ_EXT))
TEST_EXES := $(addprefix tests/,$(addsuffix $(TEST_EXE_EXT),$(TEST_NAMES)))

PROTOBUF_CXXFLAGS := $(shell $(PKG_CONFIG) --cflags protobuf 2>/dev/null)
PROTOBUF_LDLIBS := $(shell $(PKG_CONFIG) --libs protobuf 2>/dev/null)
MONGO_CXXFLAGS := $(shell $(PKG_CONFIG) --cflags libmongocxx libbsoncxx 2>/dev/null)
MONGO_LDLIBS := $(shell $(PKG_CONFIG) --libs libmongocxx libbsoncxx 2>/dev/null)
TEST_CXXFLAGS := $(CXXFLAGS) $(shell $(PKG_CONFIG) --cflags gtest 2>/dev/null)
TEST_LDFLAGS :=
TEST_LDLIBS := $(shell $(PKG_CONFIG) --libs gtest 2>/dev/null)

all: check-linux-deps $(TARGET)
test: check-linux-test-deps $(TEST_EXES)
test-resource: check-linux-test-deps tests/ResourceManagerTest$(TEST_EXE_EXT)
test-petition: check-linux-test-deps tests/PetitionManagerTest$(TEST_EXE_EXT)
test-game-service: check-linux-test-deps tests/GameServiceTest$(TEST_EXE_EXT)

run-tests: $(TEST_EXES)
	$(foreach test_exe,$(TEST_EXES),./$(test_exe) &&) true

test-help:
	@echo Build from WSL/Linux:
	@echo "  make"
	@echo "  make test"
	@echo "  make test-resource"
	@echo "  make test-petition"
	@echo "  make test-game-service"
	@echo
	@echo Expected tools:
	@echo "  g++"
	@echo "  make"
	@echo "  pkg-config"
	@echo "  protoc"
	@echo "  protobuf"
	@echo "  libmongocxx"
	@echo "  libbsoncxx"
	@echo "  gtest"

check-linux-deps:
	@command -v $(CXX) >/dev/null 2>&1 || { echo "Missing compiler: $(CXX)"; exit 1; }
	@command -v $(PROTOC) >/dev/null 2>&1 || { echo "Missing protoc: $(PROTOC)"; exit 1; }
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo "Missing pkg-config: $(PKG_CONFIG)"; exit 1; }
	@$(PKG_CONFIG) --exists protobuf || { echo "Missing protobuf package for pkg-config"; exit 1; }
	@$(PKG_CONFIG) --exists libmongocxx || { echo "Missing libmongocxx package for pkg-config"; exit 1; }
	@$(PKG_CONFIG) --exists libbsoncxx || { echo "Missing libbsoncxx package for pkg-config"; exit 1; }

check-linux-test-deps:
	@command -v $(CXX) >/dev/null 2>&1 || { echo "Missing compiler: $(CXX)"; exit 1; }
	@command -v $(PROTOC) >/dev/null 2>&1 || { echo "Missing protoc: $(PROTOC)"; exit 1; }
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo "Missing pkg-config: $(PKG_CONFIG)"; exit 1; }
	@$(PKG_CONFIG) --exists protobuf || { echo "Missing protobuf package for pkg-config"; exit 1; }
	@$(PKG_CONFIG) --exists gtest || { echo "Missing gtest package for pkg-config"; exit 1; }

$(PROTO_GEN_CC) $(PROTO_GEN_H): $(PROTO_FILE)
	$(PROTOC) --proto_path=$(PROTO_SRC_DIR) --cpp_out=. $(PROTO_FILE)

$(PROTO_OBJ): $(PROTO_GEN_CC) $(PROTO_GEN_H)
	$(CXX) $(CXXFLAGS) $(PROTOBUF_CXXFLAGS) -c $(PROTO_GEN_CC) -o $(PROTO_OBJ)

$(TARGET): $(PROTO_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) $(PROTOBUF_CXXFLAGS) $(MONGO_CXXFLAGS) -o $(TARGET) $(PROTO_OBJ) $(OBJS) $(PROTOBUF_LDLIBS) $(MONGO_LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PROTOBUF_CXXFLAGS) $(MONGO_CXXFLAGS) -c $< -o $@

tests/ResourceManagerTest$(TEST_EXE_EXT): tests/ResourceManagerTest.cpp $(TEST_MAIN) domain/Resource.cpp services/ResourceManager.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_MAIN) tests/ResourceManagerTest.cpp domain/Resource.cpp services/ResourceManager.cpp -o $@ $(TEST_LDFLAGS) $(TEST_LDLIBS)

tests/PetitionManagerTest$(TEST_EXE_EXT): tests/PetitionManagerTest.cpp $(TEST_MAIN) domain/Building.cpp domain/Petition.cpp services/PetitionManager.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS)
	$(CXX) $(TEST_CXXFLAGS) $(TEST_MAIN) tests/PetitionManagerTest.cpp domain/Building.cpp domain/Petition.cpp services/PetitionManager.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS) -o $@ $(TEST_LDFLAGS) $(TEST_LDLIBS)

tests/GameServiceTest$(TEST_EXE_EXT): tests/GameServiceTest.cpp $(TEST_MAIN) api_types.pb.cc domain/Building.cpp domain/City.cpp domain/Petition.cpp domain/Resource.cpp services/ResourceManager.cpp services/PetitionManager.cpp services/GameService.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS)
	$(CXX) $(TEST_CXXFLAGS) $(PROTOBUF_CXXFLAGS) $(MONGO_CXXFLAGS) $(TEST_MAIN) tests/GameServiceTest.cpp api_types.pb.cc domain/Building.cpp domain/City.cpp domain/Petition.cpp domain/Resource.cpp services/ResourceManager.cpp services/PetitionManager.cpp services/GameService.cpp domain/buildings/BuildingFactory.cpp $(TEST_BUILDING_SRCS) -o $@ $(TEST_LDFLAGS) $(TEST_LDLIBS) $(PROTOBUF_LDLIBS) $(MONGO_LDLIBS)

clean:
	$(DEL) $(call FIXPATH,$(OBJS)) $(TARGET) $(PROTO_GEN_CC) $(PROTO_GEN_H) $(PROTO_OBJ)

clean-tests:
	$(DEL) $(call FIXPATH,$(TEST_EXES))

re: clean all

else

all:
	@echo Backend build is configured for WSL/Linux only.
	@echo Open WSL, go to /mnt/c/Users/katie/Documents/Master-P2-EcoCity-DEV/game-engine and run: make
	@exit /b 1

test:
	@echo Backend test build is configured for WSL/Linux only.
	@echo Open WSL, go to /mnt/c/Users/katie/Documents/Master-P2-EcoCity-DEV/game-engine and run: make test
	@exit /b 1

test-resource:
	@echo Use WSL: make test-resource
	@exit /b 1

test-petition:
	@echo Use WSL: make test-petition
	@exit /b 1

test-game-service:
	@echo Use WSL: make test-game-service
	@exit /b 1

run-tests:
	@echo Use WSL: make run-tests
	@exit /b 1

test-help:
	@echo This Makefile now builds the backend only from WSL/Linux.
	@echo.
	@echo In WSL run:
	@echo "  cd /mnt/c/Users/katie/Documents/Master-P2-EcoCity-DEV/game-engine"
	@echo "  make"
	@echo "  make test"

clean:
	@echo Run clean from WSL/Linux.

clean-tests:
	@echo Run clean-tests from WSL/Linux.

re:
	@echo Run re from WSL/Linux.

endif

.PHONY: all clean clean-tests re test test-resource test-petition test-game-service run-tests test-help check-linux-deps check-linux-test-deps
