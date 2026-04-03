CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
TARGET := eco_city_engine

PROTO_SRC_DIR := ../game_api
PROTO_FILE := $(PROTO_SRC_DIR)/api.proto

PROTO_GEN_CC := api.pb.cc
PROTO_GEN_H := api.pb.h
PROTO_OBJ := api.pb.o

SRCS := main.cpp \
        $(PROTO_GEN_CC) \
        domain/City.cpp \
        domain/Petition.cpp \
        domain/Resource.cpp \
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
