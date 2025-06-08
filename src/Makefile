ALUNO = GustavoFreire
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fstack-protector-all
OBJ_DIR = ../output
PROJECT_PATH := $(shell pwd)

# Find all .c files in the current directory (src)
SRC_FILES := $(wildcard *.c)
# Generate corresponding .o file names
OBJ_FILES := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Main executable (will be created in parent directory)
PROJ_NAME = ted

# Create obj and result directories if they don't exist
$(shell mkdir -p $(OBJ_DIR))

# Default PROJ_NAME
all: $(PROJ_NAME)

# Link object files to create executable
$(PROJ_NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compile source files into object files
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean PROJ_NAME
clean:
	rm -rf $(OBJ_DIR) $(PROJ_NAME)
	rm -rf $(RESULT_DIR)
	rm -rf ../$(ALUNO).zip

pack: $(PROJ_NAME)
	cd .. && rm -f ../$(ALUNO).zip
	cd .. && echo $(ALUNO)
	cd .. && date >> .entrega
	cd .. && zip $(ALUNO).zip -r src/*.c src/*.h src/Makefile LEIA-.txt .entrega

.PHONY: all clean pack

# Run all tests (with and without .qry files)
#test-suecia: $(PROJ_NAME)
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "blow.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-1clones.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-2clones.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-3disp.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone-clones.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "clone.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "cmflg.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "cmflg-rect.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "sel-0-r1.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "sel-1-r1-r2.qry" -pr 10
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "spy-rect.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "spy.qry"
#	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/band-suecia" -f "01-band-suecia.geo" -q "transp.qry"

# test-vaso: $(PROJ_NAME)
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "blow.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "clone.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "cmflg.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-0-retang.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-1-txt.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-3-retang-regiao.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "disp-4-txt-regiao.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "spy.qry"
# 	cd .. && ./ted -o "./${RESULT_DIR}" -e "./input/vaso" -f "00-vaso-simples.geo" -q "transp.qry"