CXX := g++
SANITIZERS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
LD := gcc
NASM := nasm


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

NASMFLAGS := -f elf64 
LDFLAGS := -lm -no-pie

INC_DIRS := headers/backend_h headers/middleend_h headers/frontend_h headers/processor_h headers/common_h  headers/backend-x64_h
INCLUDES = $(addprefix -I,$(INC_DIRS))

SRC_DIR := source
FRONTEND_DIR := $(SRC_DIR)/frontend
BACKEND_DIR := $(SRC_DIR)/backend
BACKEND_X64_DIR := $(SRC_DIR)/backend_x64
MIDDLEEND_DIR := $(SRC_DIR)/middleend
COMMON_DIR := $(SRC_DIR)/common
PROCESSOR_DIR := $(SRC_DIR)/processor

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

FRONTEND_PROG := $(BIN_DIR)/frontend
MIDDLEEND_PROG := $(BIN_DIR)/middleend  
BACKEND_PROG := $(BIN_DIR)/backend
BACKEND_X64_PROG := $(BIN_DIR)/backend_x64

COMMON_SOURCES :=   $(wildcard $(FRONTEND_DIR)/*.cpp) \
                    $(wildcard $(COMMON_DIR)/*.cpp) \
					$(wildcard $(MIDDLEEND_DIR)/*.cpp) \
					$(wildcard $(BACKEND_DIR)/*.cpp) \
					$(wildcard $(BACKEND_X64_DIR)/*.cpp) \

FRONTEND_SOURCES := exe/frontend.cpp \
                    $(COMMON_SOURCES) \

MIDDLEEND_SOURCES := exe/middleend.cpp \
                    $(COMMON_SOURCES) \

BACKEND_SOURCES := exe/backend.cpp \
					$(COMMON_SOURCES)\
				    $(wildcard $(PROCESSOR_DIR)/*.cpp) \

BACKEND_X64_SOURCES := exe/backend-x64.cpp \
                    $(COMMON_SOURCES) \

FRONTEND_OBJECTS := $(addprefix $(OBJ_DIR)/,$(notdir $(FRONTEND_SOURCES:.cpp=.o)))
MIDDLEEND_OBJECTS := $(addprefix $(OBJ_DIR)/,$(notdir $(MIDDLEEND_SOURCES:.cpp=.o)))
BACKEND_OBJECTS := $(addprefix $(OBJ_DIR)/,$(notdir $(BACKEND_SOURCES:.cpp=.o)))
BACKEND_X64_OBJECTS := $(addprefix $(OBJ_DIR)/,$(notdir $(BACKEND_X64_SOURCES:.cpp=.o)))

all: $(FRONTEND_PROG) $(MIDDLEEND_PROG) $(BACKEND_PROG) $(BACKEND_X64_PROG)

$(FRONTEND_PROG): $(FRONTEND_OBJECTS) | $(BIN_DIR)
	@$(CXX) $(FRONTEND_OBJECTS) $(SANITIZERS) -o $@

$(MIDDLEEND_PROG): $(MIDDLEEND_OBJECTS) | $(BIN_DIR)
	@$(CXX) $(MIDDLEEND_OBJECTS) $(SANITIZERS) -o $@

$(BACKEND_PROG): $(BACKEND_OBJECTS) | $(BIN_DIR)
	@$(CXX) $(BACKEND_OBJECTS) $(SANITIZERS) -o $@

$(BACKEND_X64_PROG): $(BACKEND_X64_OBJECTS) | $(BIN_DIR)
	@$(CXX) $(BACKEND_X64_OBJECTS) $(SANITIZERS) -o $@

frontend: $(FRONTEND_PROG)
middleend: $(MIDDLEEND_PROG)
backend: $(BACKEND_PROG)
backend_x64: $(BACKEND_X64_PROG)

run: all
	@./$(FRONTEND_PROG)
	@./$(MIDDLEEND_PROG)
	@./$(BACKEND_X64_PROG)

run_frontend:
	@./$(FRONTEND_PROG)

run_middleend:
	@./$(MIDDLEEND_PROG)

run_backend:
	@./$(BACKEND_PROG)

run_backend_x64:
	@./$(BACKEND_X64_PROG)


$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/*/%.cpp | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: exe/%.cpp | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)


OUTPUT_DIR := output
NASM_PROG := $(OUTPUT_DIR)/nasm_output

nasm: 
	@nasm -f elf64 $(OUTPUT_DIR)/nasm_output.asm -o $(OUTPUT_DIR)/nasm_output.o
	@gcc $(OUTPUT_DIR)/nasm_output.o -o $(NASM_PROG) -lm -no-pie
	@./$(NASM_PROG)



nasm_clean:
	@rm -f $(OUTPUT_DIR)/nasm_output.asm $(OUTPUT_DIR)/nasm_output.o $(OUTPUT_DIR)/nasm_output


logger-clean:
	rm -rf logger/
	rm -rf output/
	mkdir -p logger/pics logger/dots
	mkdir -p output/

rebuild: clean  logger-clean all

.PHONY: all clean rebuild frontend middleend backend run logger-clean

