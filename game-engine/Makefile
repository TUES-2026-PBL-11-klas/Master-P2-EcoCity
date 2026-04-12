CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
TARGET := eco_city_engine

PROTOC := C:/vcpkg/installed/x64-mingw-dynamic/tools/protobuf/protoc.exe

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
else
    OBJ_EXT := .o
endif

OBJS := $(SRCS:.cpp=$(OBJ_EXT))

OBJS += $(BUILDING_SRCS:.cpp=$(OBJ_EXT))

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
else
    DEL := rm -f
    FIXPATH = $1
    LDFLAGS :=
    LIBS := `pkg-config --libs protobuf` \
            `pkg-config --libs libmongocxx` \
            `pkg-config --libs libbsoncxx`
    CXXFLAGS += `pkg-config --cflags libmongocxx` \
                `pkg-config --cflags libbsoncxx`
endif

all: $(TARGET)

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

clean:
	$(DEL) $(call FIXPATH,$(OBJS)) $(TARGET) $(PROTO_GEN_CC) $(PROTO_GEN_H) $(PROTO_OBJ)
ifeq ($(OS),Windows_NT)
	-$(DEL) *.pdb *.ilk
endif

re: clean all

.PHONY: all clean re
