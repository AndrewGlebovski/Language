# Путь к компилятору
COMPILER=g++

# Флаги компиляции
FLAGS=-Wno-unused-parameter -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_

# Папка с объектами
BIN_DIR=binary

# Папка с исходниками и заголовками
SRC_DIR=source

# Папка с исходниками и заголовками
LIB_DIR=$(SRC_DIR)/libs

# Зависимости frontend.cpp
FRONT_DPD=tree


all: frontend


# Завершает сборку frontend.cpp
frontend: $(addprefix $(BIN_DIR)/, $(addsuffix .o, frontend $(FRONT_DPD)))
	$(COMPILER) $^ -o front.exe


# Предварительная сборка frontend.cpp
$(BIN_DIR)/frontend.o: $(addprefix $(SRC_DIR)/, frontend.cpp frontend.hpp) $(addprefix $(LIB_DIR)/, $(addsuffix .hpp, $(FRONT_DPD)))
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка библиотек
$(BIN_DIR)/%.o: $(addprefix $(LIB_DIR)/, %.cpp %.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@
