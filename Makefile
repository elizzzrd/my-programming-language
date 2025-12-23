CXX := g++
SANITIZERS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr


CXXFLAGS := -g -DDEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations \
 -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported \
 -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness\
 -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith\
 -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2\
 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default\
 -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix \
 -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new \
 -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 \
 -Wstack-usage=8192 -pie -fPIE -Werror=vla 

INC_DIRS := headers/backend_h headers/middleend_h headers/frontend_h headers/processor_h headers/common_h headers/processor
INCLUDES = $(addprefix -I,$(INC_DIRS))

SRC_DIR := source
FRONTEND_DIR := $(SRC_DIR)/frontend
BACKEND_DIR := $(SRC_DIR)/backend
MIDDLEEND_DIR := $(SRC_DIR)/middleend
COMMON_DIR := $(SRC_DIR)/common
PROCESSOR_DIR := $(SRC_DIR)/processor

HEADERS_DIR := headers
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj

SOURCES := main.cpp \
          $(wildcard $(FRONTEND_DIR)/*.cpp) \
		  $(wildcard $(BACKEND_DIR)/*.cpp) \
		  $(wildcard $(MIDDLEEND_DIR)/*.cpp) \
		  $(wildcard $(COMMON_DIR)/*.cpp)  \
		  $(wildcard $(PROCESSOR_DIR)/*.cpp)  \

OBJECTS := $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)

TARGET := $(BIN_DIR)/main

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@$(CXX) $(OBJECTS) $(SANITIZERS) -o $@ 

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

logger-clean:
	rm -rf logger/*
	mkdir -p logger


rebuild: clean all


check: 
	cd build/bin
	./$(TARGET)


.PHONY: all clean rebuild start