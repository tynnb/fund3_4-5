CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic -fsanitize=address -std=c99
LDFLAGS = -fsanitize=address

PROGRAM = main
TEST_PROGRAM = tests

SOURCES = main.c funcs.c
TEST_SOURCES = test.c funcs.c

OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

all: $(PROGRAM) $(TEST_PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS)

$(TEST_PROGRAM): $(TEST_OBJECTS)
	$(CC) $(LDFLAGS) -o $(TEST_PROGRAM) $(TEST_OBJECTS)

main.o: main.c funcs.h
	$(CC) $(CFLAGS) -c main.c

funcs.o: funcs.c funcs.h
	$(CC) $(CFLAGS) -c funcs.c

test.o: test.c funcs.h
	$(CC) $(CFLAGS) -c test.c

test: $(TEST_PROGRAM)
	@echo "=== Running tests ==="
	./$(TEST_PROGRAM)

run: $(PROGRAM)
	@echo "=== Running program ==="
	./$(PROGRAM)

debug: $(PROGRAM)
	valgrind --leak-check=full --track-origins=yes ./$(PROGRAM)

debug-test: $(TEST_PROGRAM)
	valgrind --leak-check=full --track-origins=yes ./$(TEST_PROGRAM)

fast:
	$(CC) -Wall -std=c99 -o $(PROGRAM) main.c funcs.c
	$(CC) -Wall -std=c99 -o $(TEST_PROGRAM) test.c funcs.c

clean:
	rm -f $(PROGRAM) $(TEST_PROGRAM) *.o

format:
	clang-format -i *.c *.h

.PHONY: all test run debug debug-test fast clean format