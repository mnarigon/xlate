#
# -- makefile
#

# -- general configuration
SHELL=/bin/sh
# -- clear implicit targets
.SUFFIXES:

# -- program extension
ifeq ($(OS),Windows_NT)
    EXE=".exe"
else
    EXE=
endif

#
# -- directories
SOURCE_DIR=     .
COMMON_DIR=     ../common
TEST_DIR=       ../test
BUILD_DIR=      build
DEPENDENCY_DIR= build/.d
INSTALL_DIR=    /usr/local/bin

#
# -- compiler flags
CC= cc
CCFLAGS= -O

#
# -- dependency generation flags
DEPFLAGS= -MT $@ -MMD -MP -MF $(DEPENDENCY_DIR)/$*.Td

#
# -- linker flags
LDFLAGS=

#
# -- program name
PROGRAM= xlate-z80$(EXE)

#
# -- name all the source files which need to be compiled
SOURCES= \
	main.c \
	lookup.c \
	translate.c

COMMON= \
	buffer.c \
	pmark.c \
	scanner.c \
	warning.c

#
# -- name all the resulting object files
OBJECTS= $(SOURCES:%.c=$(BUILD_DIR)/%.o) $(COMMON:%.c=$(BUILD_DIR)/%.o)

#
# -- make all target
all: $(BUILD_DIR) $(BUILD_DIR)/$(PROGRAM)

#
# -- make build directory target
$(BUILD_DIR):
	@echo "Creating build directories ..."
	@/bin/mkdir $(BUILD_DIR)
	@/bin/mkdir $(DEPENDENCY_DIR)

#
# -- compile a source file target
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(DEPENDENCY_DIR)/%.d
	-@echo "Compiling $< ..."
	@$(CC) $(CCFLAGS) $(DEPFLAGS) -I$(SOURCE_DIR) -I$(COMMON_DIR) -c $< -o $@
	-@/bin/mv -f $(DEPENDENCY_DIR)/$*.Td $(DEPENDENCY_DIR)/$*.d

#
# -- compile a common file target
$(BUILD_DIR)/%.o: $(COMMON_DIR)/%.c $(DEPENDENCY_DIR)/%.d
	-@echo "Compiling $< ..."
	@$(CC) $(CCFLAGS) $(DEPFLAGS) -I$(SOURCE_DIR) -I$(COMMON_DIR) -c $< -o $@
	-@/bin/mv -f $(DEPENDENCY_DIR)/$*.Td $(DEPENDENCY_DIR)/$*.d

#
# -- link the program target
$(BUILD_DIR)/$(PROGRAM): $(BUILD_DIR) $(OBJECTS)
	-@/bin/rm -f $@
	@echo "Linking $@ ..."
	@$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

#
# -- clean build directory target
clean:
	@echo "Removing build directories ..."
	-@/bin/rm -rf $(BUILD_DIR) core

#
# -- install target
install: $(BUILD_DIR)/$(PROGRAM)
	@echo "Installing $(PROGRAM) ..."
	@/bin/cp -f $(BUILD_DIR)/$(PROGRAM) $(INSTALL_DIR)/$(PROGRAM)
	@/bin/chmod 755 $(INSTALL_DIR)/$(PROGRAM)

#
# -- run test files
check: $(BUILD_DIR)/$(PROGRAM)
	@echo "Checking $(PROGRAM) ..."
	$(BUILD_DIR)/$(PROGRAM) $(TEST_DIR)/z80.z80 > $(TEST_DIR)/z80.z80.out
	$(BUILD_DIR)/$(PROGRAM) $(TEST_DIR)/syntax.z80 > $(TEST_DIR)/syntax.z80.out

#
# -- dependency directory
$(DEPENDENCY_DIR)/%.d: ;
.PRECIOUS: $(DEPENDENCY_DIR)/%.d

#
# -- include compiler generated dependency information
-include $(patsubst %,$(DEPENDENCY_DIR)/%.d,$(basename $(SOURCES)))
