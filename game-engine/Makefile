CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
TARGET := eco_city_engine

PROTOC := C:/vcpkg/installed/x64-mingw-dynamic/tools/protobuf/protoc.exe

PROTO_SRC_DIR := ../game_api/v1
PROTO_FILE := $(PROTO_SRC_DIR)/api_types.proto

PROTO_GEN_CC := api_types.pb.cc
PROTO_GEN_H := api_types.pb.h
PROTO_OBJ := api_types.pb.o

SRCS := main.cpp \
		domain/Building.cpp \
		domain/buildings/BuildingFactory.cpp \
        domain/City.cpp \
        domain/Petition.cpp \
        domain/Resource.cpp \
        services/GameService.cpp \
        services/PetitionManager.cpp \
        services/ResourceManager.cpp

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

OBJS := $(SRCS:.cpp=.o)

OBJS += $(BUILDING_SRCS:.cpp=.o)

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    DEL := del /q
    FIXPATH = $(subst /,\,$1)
    VCPKG_LIB := C:/vcpkg/installed/x64-mingw-dynamic/lib
    CXXFLAGS += -IC:/vcpkg/installed/x64-mingw-dynamic/include
    LDFLAGS := -L$(VCPKG_LIB)
    LIBS := -lprotobuf -lprotobuf-lite -labseil_dll -lutf8_validity -lws2_32
else
    DEL := rm -f
    FIXPATH = $1
	LDFLAGS :=
    LIBS := `pkg-config --libs protobuf`
endif

all: $(TARGET)

$(PROTO_GEN_CC) $(PROTO_GEN_H): $(PROTO_FILE)
	$(PROTOC) --proto_path=$(PROTO_SRC_DIR) --cpp_out=. $(PROTO_FILE)

$(PROTO_OBJ): $(PROTO_GEN_CC) $(PROTO_GEN_H)
	$(CXX) $(CXXFLAGS) -c $(PROTO_GEN_CC) -o $(PROTO_OBJ)

$(TARGET): $(PROTO_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(PROTO_OBJ) $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(DEL) $(call FIXPATH,$(OBJS)) $(TARGET) $(PROTO_GEN_CC) $(PROTO_GEN_H) $(PROTO_OBJ)

re: clean all

.PHONY: all clean re
