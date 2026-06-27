# ============================================================
# Компиляторы и флаги
# ============================================================
CXX := g++
CC := gcc
NASM := nasm

# Санитайзеры (включены, но можно отключить для релиза)
SANITIZERS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

# Флаги компиляции
CXXFLAGS := -g -DDEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ \
    -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations \
    -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported \
    -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal \
    -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 \
    -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual \
    -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow \
    -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 \
    -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types \
    -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef \
    -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix \
    -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast \
    -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector \
    -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer \
    -Wlarger-than=8192 -Wstack-usage=8192 -Werror=vla

# Исправление: убираем -pie (оставляем только -no-pie в LDFLAGS)
CXXFLAGS += -fno-pie

# Флаги для ассемблера
NASMFLAGS := -f elf64 -g -F dwarf

# Флаги линковки
LDFLAGS := -lm -no-pie

# ============================================================
# Директории
# ============================================================
SRC_DIR := source
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

# Поддиректории исходников
FRONTEND_DIR := $(SRC_DIR)/frontend
BACKEND_DIR := $(SRC_DIR)/backend
BACKEND_X64_DIR := $(SRC_DIR)/backend_x86_64
MIDDLEEND_DIR := $(SRC_DIR)/middleend
COMMON_DIR := $(SRC_DIR)/common
PROCESSOR_DIR := $(SRC_DIR)/processor
EXE_DIR := exe

# Директории заголовков
INC_DIRS := headers/backend_h headers/middleend_h headers/frontend_h \
            headers/processor_h headers/common_h headers/backend-x64_h
INCLUDES := $(addprefix -I,$(INC_DIRS))

# ============================================================
# Исполняемые файлы
# ============================================================
FRONTEND_PROG := $(BIN_DIR)/frontend
MIDDLEEND_PROG := $(BIN_DIR)/middleend
BACKEND_PROG := $(BIN_DIR)/backend
BACKEND_X64_PROG := $(BIN_DIR)/backend_x64

# ============================================================
# Исходные файлы (все .cpp из поддиректорий)
# ============================================================
COMMON_SOURCES := $(wildcard $(FRONTEND_DIR)/*.cpp) \
                  $(wildcard $(COMMON_DIR)/*.cpp) \
                  $(wildcard $(MIDDLEEND_DIR)/*.cpp) \
                  $(wildcard $(BACKEND_DIR)/*.cpp) \
                  $(wildcard $(BACKEND_X64_DIR)/*.cpp)

# Убираем дубликаты
COMMON_SOURCES := $(sort $(COMMON_SOURCES))

# Списки исходников для каждого исполняемого файла
FRONTEND_SOURCES := $(EXE_DIR)/frontend.cpp $(COMMON_SOURCES)
MIDDLEEND_SOURCES := $(EXE_DIR)/middleend.cpp $(COMMON_SOURCES)
BACKEND_SOURCES := $(EXE_DIR)/backend.cpp $(COMMON_SOURCES) \
                   $(wildcard $(PROCESSOR_DIR)/*.cpp)
BACKEND_X64_SOURCES := $(EXE_DIR)/backend-x64.cpp $(COMMON_SOURCES)

# ============================================================
# Функция: преобразование .cpp в .o с сохранением структуры
# ============================================================
define cpp_to_obj
$(OBJ_DIR)/$(patsubst $(SRC_DIR)/%,%,$(patsubst $(EXE_DIR)/%,exe/%,$(1:.cpp=.o)))
endef

# Преобразование списков исходников в объектные файлы
FRONTEND_OBJECTS := $(foreach src,$(FRONTEND_SOURCES),$(call cpp_to_obj,$(src)))
MIDDLEEND_OBJECTS := $(foreach src,$(MIDDLEEND_SOURCES),$(call cpp_to_obj,$(src)))
BACKEND_OBJECTS := $(foreach src,$(BACKEND_SOURCES),$(call cpp_to_obj,$(src)))
BACKEND_X64_OBJECTS := $(foreach src,$(BACKEND_X64_SOURCES),$(call cpp_to_obj,$(src)))

# ============================================================
# Правила сборки
# ============================================================

# Цель по умолчанию
all: $(FRONTEND_PROG) $(MIDDLEEND_PROG) $(BACKEND_PROG) $(BACKEND_X64_PROG)

# Сборка исполняемых файлов
$(FRONTEND_PROG): $(FRONTEND_OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CXX) $^ $(SANITIZERS) $(LDFLAGS) -o $@

$(MIDDLEEND_PROG): $(MIDDLEEND_OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CXX) $^ $(SANITIZERS) $(LDFLAGS) -o $@

$(BACKEND_PROG): $(BACKEND_OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CXX) $^ $(SANITIZERS) $(LDFLAGS) -o $@

$(BACKEND_X64_PROG): $(BACKEND_X64_OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CXX) $^ $(SANITIZERS) $(LDFLAGS) -o $@

# ============================================================
# Правила компиляции
# ============================================================

# Правило для файлов из exe/ (компилируются в obj/exe/)
$(OBJ_DIR)/exe/%.o: $(EXE_DIR)/%.cpp | $(OBJ_DIR)/exe
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Правило для файлов из source/ (сохраняет структуру)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Создание директорий
$(BIN_DIR):
	@mkdir -p $@

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/exe:
	@mkdir -p $@

# ============================================================
# Запуск
# ============================================================

frontend: $(FRONTEND_PROG)
middleend: $(MIDDLEEND_PROG)
backend: $(BACKEND_PROG)
backend_x64: $(BACKEND_X64_PROG)

run: all
	@echo "=== Running frontend ==="
	@./$(FRONTEND_PROG)
	@echo "=== Running middleend ==="
	@./$(MIDDLEEND_PROG)
	@echo "=== Running backend_x64 ==="
	@./$(BACKEND_X64_PROG)

run_frontend:
	@./$(FRONTEND_PROG)

run_middleend:
	@./$(MIDDLEEND_PROG)

run_backend:
	@./$(BACKEND_PROG)

run_backend_x64:
	@./$(BACKEND_X64_PROG)

# ============================================================
# Сборка NASM-программы
# ============================================================

OUTPUT_DIR := output
NASM_PROG := $(OUTPUT_DIR)/nasm_output
ASM_FILE := $(OUTPUT_DIR)/nasm_output.asm

nasm: $(ASM_FILE)
	@echo "Assembling $(ASM_FILE)..."
	@$(NASM) $(NASMFLAGS) $(ASM_FILE) -o $(OUTPUT_DIR)/nasm_output.o
	@echo "Linking NASM program..."
	@$(CC) $(OUTPUT_DIR)/nasm_output.o -o $(NASM_PROG) -lm -no-pie
	@echo "Running $(NASM_PROG)..."
	@./$(NASM_PROG)

# Зависимость: если .asm обновлён, пересобираем
$(ASM_FILE): $(BACKEND_X64_PROG)
	@echo "Generating $(ASM_FILE)..."
	@./$(BACKEND_X64_PROG)

nasm_clean:
	@rm -f $(OUTPUT_DIR)/nasm_output.asm $(OUTPUT_DIR)/nasm_output.o $(OUTPUT_DIR)/nasm_output

# ============================================================
# Очистка и обслуживание
# ============================================================

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned."

logger-clean:
	@rm -rf logger/ output/
	@mkdir -p logger/pics logger/dots
	@mkdir -p output/
	@echo "Logger and output directories cleaned."

# Полная пересборка
rebuild: clean logger-clean all

.PHONY: all clean rebuild frontend middleend backend run logger-clean \
        nasm nasm_clean run_frontend run_middleend run_backend run_backend_x64