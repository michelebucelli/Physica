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

# LOCALIZATIONS
LOCALE=it

# BUILDS THE FILES
$(OUTPUT): $(SOURCE) $(HEADERS)
	@echo "Building"
	@$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCE) $(LINKING)

build: $(OUTPUT)

# BUILDS THE LOCALIZATION FILES FOR A SPECIFIED LOCALE
# run like:
#   make LOCALE=en locale
bin/data/locale/$(LOCALE)/LC_MESSAGES/physica-core.mo: bin/data/locale/$(LOCALE)/physica-core.po
	@echo "Building localizations: " $(LOCALE)
	@msgfmt bin/data/locale/$(LOCALE)/physica-core.po -obin/data/locale/$(LOCALE)/LC_MESSAGES/physica-core.mo

locale: bin/data/locale/$(LOCALE)/LC_MESSAGES/physica-core.mo

# RUNS THE EXECUTABLE
run:
	@echo "Running"
	@ (cd bin && ./exe && cat output.log)
	
# RUNS WITH GDB FOR DEBUG
debug:
	@echo "Running with GDB"
	@ (cd bin && gdb exe --eval-command run --eval-command quit && cat output.log)
	
# MAKES EVERYTHING
all: locale build run
