CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
TARGET := eco_city_engine

PROTO_SRC_DIR := ../game_api
PROTO_FILE := $(PROTO_SRC_DIR)/v1/api_types.proto

PROTO_GEN_CC := v1/api_types.pb.cc
PROTO_GEN_H := v1/api_types.pb.h
PROTO_OBJ := v1/api_types.pb.o

SRCS := main.cpp \
        $(PROTO_GEN_CC) \
        domain/Building.cpp \
        domain/City.cpp \
        domain/Petition.cpp \
        domain/Resource.cpp \
        domain/buildings/AirportExpansion.cpp \
        domain/buildings/BuildingFactory.cpp \
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
        services/GameService.cpp \
        services/PetitionManager.cpp \
        services/ResourceManager.cpp

OBJS := $(SRCS:.cpp=.o)

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    DEL := del /q
    FIXPATH = $(subst /,\,$1)
    LIBS := -lprotobuf
else
    DEL := rm -f
    FIXPATH = $1
    LIBS := `pkg-config --libs protobuf`
endif

all: $(TARGET)

$(PROTO_GEN_CC) $(PROTO_GEN_H): $(PROTO_FILE)
	protoc --proto_path=$(PROTO_SRC_DIR) --cpp_out=. $(PROTO_FILE)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(DEL) $(call FIXPATH,$(OBJS)) $(TARGET) $(PROTO_GEN_CC) $(PROTO_GEN_H)

re: clean all

.PHONY: all clean re
