SHELL_NAME = $(shell uname -s)

ifeq ($(SHELL_NAME), Linux)
COMMAND = gcc

.PHONY: all
all: gen_numbers.out select_arg.out gen_in_range.out

%.out: *.h *.c
	$(COMMAND) -o select_arg.out select_arg.c utils.c
	$(COMMAND) -o gen_in_range.out gen_in_range.c
	$(COMMAND) -o gen_numbers.out gen_numbers.c utils.c

test: *.h *.c
	$(COMMAND) -o select_arg.out select_arg.c utils.c
	$(COMMAND) -o gen_in_range.out gen_in_range.c
	$(COMMAND) -o gen_numbers.out gen_numbers.c utils.c
	./select_arg.out 1 15 && echo "select_arg.out returns 0." || echo "Error: requires two numbers."
	./gen_in_range.out 1 15 && echo "gen_in_range.out returns 0." || echo "Error: requires two numbers and second number must be greater than first number."
	./gen_numbers.out 1 15 && echo "gen_numbers.out returns 0." || echo "Error: requires two numbers and both must be above 0."

.PHONY: clean
clean:
	rm -f *.out

else
.PHONY: all
all:
	@echo "Sorry, I prefer Linux"
endif