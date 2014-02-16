# COMPILER
CC= g++

# COMPILER FLAGS
CFLAGS= -w

# LINKER OPTIONS
LINKING= -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_ttf

# SOURCE FILES
SOURCE= src/*.cpp src/TinyJS/*.cpp src/PugiXML/*.cpp

# HEADER FILES
HEADERS= src/*.h src/TinyJS/*.h src/PugiXML/*.hpp

# OUTPUT
OUTPUT= bin/exe

$(OUTPUT): $(SOURCE) $(HEADERS)
	@echo "Building"
	@$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCE) $(LINKING)

build: $(OUTPUT)
	
run:
	@echo "Running"
	@ (cd bin && ./exe && cat output.log)
	
debug:
	@echo "Running with GDB"
	@ (cd bin && gdb exe --eval-command run --eval-command quit && cat output.log)
