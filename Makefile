# Путь к компилятору
COMPILER=g++

# Флаги компиляции
FLAGS=-Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_

# Папка с объектами
BIN_DIR=binary

# Папка с исходниками и заголовками
SRC_DIR=source

# Папка с исходниками и заголовками
LIB_DIR=$(SRC_DIR)/libs


all: front


# Завершает сборку front.cpp
front: $(addprefix $(BIN_DIR)/, $(addsuffix .o, front image_parser symbol_parser tree))
	$(COMPILER) $^ -o front.exe


# Предварительная сборка front.cpp
$(BIN_DIR)/front.o: $(addprefix $(SRC_DIR)/, front.cpp symbol_parser.hpp image_parser.hpp) $(addprefix $(LIB_DIR)/, tree.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка image_parser.cpp
$(BIN_DIR)/image_parser.o: $(addprefix $(SRC_DIR)/, image_parser.cpp image_parser.hpp stb_image.h)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка symbol_parser.cpp
$(BIN_DIR)/symbol_parser.o: $(addprefix $(SRC_DIR)/, symbol_parser.cpp symbol_parser.hpp image_parser.hpp) $(addprefix $(LIB_DIR)/, tree.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка библиотек
$(BIN_DIR)/%.o: $(addprefix $(LIB_DIR)/, %.cpp %.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@
