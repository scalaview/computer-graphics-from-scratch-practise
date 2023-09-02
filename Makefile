DIR := $(subst /,\,${CURDIR})

BUILD_DIR := bin
OBJ_DIR := obj

ASSEMBLY := src
APP := render
EXTENSION := .exe
COMPILER_FLAGS := -std=c99 -g -MD -Werror=vla -Wno-missing-braces -fdeclspec
INCLUDE_FLAGS :=
LINKER_FLAGS := -g -L$(BUILD_DIR)
DEFINES := -D_DEBUG -DKIMPORT

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.c) # Get all .c files
DIRECTORIES := \$(ASSEMBLY) $(subst $(DIR),,$(shell dir $(ASSEMBLY) /S /AD /B | findstr /i src)) # Get all directories under src.
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) # Get all compiled .c.o objects for tests

all: scaffold compile link

.PHONY: scaffold
scaffold: # create build directory
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) && mkdir $(addprefix $(BUILD_DIR), $(DIRECTORIES)) 2>NUL || cd .
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # link
	@echo Linking $(APP)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)/$(APP)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile: #compile .c files
	@echo Compiling...

.PHONY: clean
clean: # clean build directory
	if exist $(BUILD_DIR)\$(APP)$(EXTENSION) del $(BUILD_DIR)\$(APP)$(EXTENSION)
	rmdir /s /q $(BUILD_DIR)
	rmdir /s /q $(OBJ_DIR)

$(OBJ_DIR)/%.c.o: %.c # compile .c to .c.o object
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

include $(OBJ_FILES:.o)
