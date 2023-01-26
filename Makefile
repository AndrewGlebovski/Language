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


all: $(BIN_DIR) front middle back


# Завершает сборку front.cpp
front: $(addprefix $(BIN_DIR)/, $(addsuffix .o, front image_parser symbol_parser grammar input-output tree text dif dsl parser))
	$(COMPILER) $^ -o front.exe


# Завершает сборку back.cpp
back: $(addprefix $(BIN_DIR)/, $(addsuffix .o, back input-output tree text program stack parser))
	$(COMPILER) $^ -o back.exe


# Завершает сборку middle.cpp
middle: $(addprefix $(BIN_DIR)/, $(addsuffix .o, middle input-output dif dsl text tree parser))
	$(COMPILER) $^ -o middle.exe


# Предварительная сборка front.cpp
$(BIN_DIR)/front.o: $(addprefix $(SRC_DIR)/, front.cpp symbol_parser.hpp image_parser.hpp grammar.hpp input-output.hpp) $(addprefix $(LIB_DIR)/, tree.hpp parser.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка back.cpp
$(BIN_DIR)/back.o: $(addprefix $(SRC_DIR)/, back.cpp input-output.hpp) $(addprefix $(LIB_DIR)/, tree.hpp parser.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка middle.cpp
$(BIN_DIR)/middle.o: $(addprefix $(SRC_DIR)/, middle.cpp input-output.hpp dif.hpp dsl.hpp) $(addprefix $(LIB_DIR)/, tree.hpp parser.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка image_parser.cpp
$(BIN_DIR)/image_parser.o: $(addprefix $(SRC_DIR)/, image_parser.cpp image_parser.hpp stb_image.h stb_image_write.h)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка symbol_parser.cpp
$(BIN_DIR)/symbol_parser.o: $(addprefix $(SRC_DIR)/, symbol_parser.cpp symbol_parser.hpp image_parser.hpp reserved_shapes.hpp) $(addprefix $(LIB_DIR)/, tree.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка grammar.cpp
$(BIN_DIR)/grammar.o: $(addprefix $(SRC_DIR)/, grammar.cpp grammar.hpp symbol_parser.hpp image_parser.hpp dif.hpp) $(addprefix $(LIB_DIR)/, tree.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка input-output.cpp
$(BIN_DIR)/input-output.o: $(addprefix $(SRC_DIR)/, input-output.cpp input-output.hpp) $(addprefix $(LIB_DIR)/, tree.hpp text.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка program.cpp
$(BIN_DIR)/program.o: $(addprefix $(SRC_DIR)/, program.cpp program.hpp) $(addprefix $(LIB_DIR)/, tree.hpp stack.hpp text.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка dif.cpp
$(BIN_DIR)/dif.o: $(addprefix $(SRC_DIR)/, dif.cpp dif.hpp dsl.hpp gen.hpp) $(addprefix $(LIB_DIR)/, tree.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка dsl.cpp
$(BIN_DIR)/dsl.o: $(addprefix $(SRC_DIR)/, dsl.cpp dsl.hpp) $(addprefix $(LIB_DIR)/, tree.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Предварительная сборка библиотек
$(BIN_DIR)/%.o: $(addprefix $(LIB_DIR)/, %.cpp %.hpp)
	$(COMPILER) $(FLAGS) -c $< -o $@


# Создание папки для объектников, если она еще не существует
$(BIN_DIR):
	mkdir $@
