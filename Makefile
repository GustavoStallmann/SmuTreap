CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fstack-protector-all
SRC_DIR = src
OBJ_DIR = output
RESULT_DIR = /home/stallmarch/projects/uel/ed2/project/result
PROJECT_PATH := $(shell pwd)

# Find all .c files in the source directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
# Generate corresponding .o file names
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Main executable
TARGET = program

# Create obj and result directories if they don't exist
$(shell mkdir -p $(OBJ_DIR))
$(shell mkdir -p $(RESULT_DIR))

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


# Clean target
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	rm -rf ${RESULT_DIR}

test:
	./program -o "./result" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "blow.qry"

# Run all tests (with and without .qry files)
test-suecia: $(TARGET)
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "blow.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-1clones.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-2clones.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-3disp.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-clones.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "cmflg.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "cmflg-rect.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "sel-0-r1.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "sel-1-r1-r2.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "spy-rect.qry" -p 2000 -hc 5 -pr 1.15
	./program -o "${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "transp.qry" -p 2000 -hc 5 -pr 1.15

test-vaso: $(TARGET)
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "blow.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "clone.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "cmflg.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "spy.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-0-retang.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-1-txt.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-3-retang-regiao.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-4-txt-regiao.qry"
	./program -o "${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "transp.qry"

debug:
	valgrind --leak-check=full --track-origins=yes ./program -o "./${RESULT_DIR}" -e "./input" -f "01-band-suecia.geo"

debug-qry:
	valgrind --leak-check=full --track-origins=yes ./program -o "./${RESULT_DIR}" -e "./input" -f "01-band-suecia.geo" -q "transp.qry"

.PHONY: all clean test-all-qry test-all debug-qry
