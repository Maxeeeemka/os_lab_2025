CC = gcc
CFLAGS = -Wall -Werror -O2

PARALLEL = parallel_min_max
MEMORY   = process_memory

PAR_OBJS = parallel_min_max.o find_min_max.o utils.o

all: $(PARALLEL) $(MEMORY)
	@echo "✅ Все программы собраны"

$(PARALLEL): $(PAR_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c parallel_min_max.c

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c find_min_max.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

$(MEMORY): process_memory.c
	$(CC) $(CFLAGS) -o $@ process_memory.c

clean:
	rm -f *.o $(PARALLEL) $(MEMORY)
	@echo "🧹 Очистка завершена"
