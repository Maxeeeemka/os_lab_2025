CC = gcc
CFLAGS = -Wall -Werror -O2
LAUNCHER = launcher

OBJS = find_min_max.o utils.o

SEQ = sequential_min_max
PAR = parallel_min_max

all: $(SEQ) $(PAR) $(LAUNCHER)
	@echo "✅ Сборка всех программ завершена!"

$(SEQ): $(OBJS) sequential_min_max.o
	$(CC) $(CFLAGS) -o $@ $^

$(PAR): $(OBJS) parallel_min_max.o
	$(CC) $(CFLAGS) -o $@ $^

$(LAUNCHER): launcher.o
	$(CC) $(CFLAGS) -o $@ $^

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c find_min_max.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

sequential_min_max.o: sequential_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c sequential_min_max.c

parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c parallel_min_max.c

launcher.o: launcher.c
	$(CC) $(CFLAGS) -c launcher.c

clean:
	rm -f *.o $(SEQ) $(PAR)
	@echo "🧹 Очистка завершена"

