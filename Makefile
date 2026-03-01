CC = gcc
CCFLAGS =  -std=c99 -O3 -D_POSIX_C_SOURCE=200809L
LDFLAGS = 
DEBUGFLAGS := -g -O0
BUILD_DIR = build
SOURCE_DIR = src
TARGET = $(BUILD_DIR)/main
SOURCES = $(shell find $(SOURCE_DIR) -name '*.c') #find all .c files inside the source directory
OBJECTS = $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o) 
DEPENDENCIES = $(OBJECTS:.o=.d) 

all: $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(CCFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile | $(BUILD_DIR) 
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(INCLUDE_DIRS) -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)

$(BUILD_DIR):
	mkdir -p $@


debug: CCFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

run:
	./$(TARGET) 

clean:
	$(RM) -r $(BUILD_DIR)

.PHONY: run clean debug