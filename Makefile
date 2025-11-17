CFLAGS = -O2 -g -std=gnu99 -Wall
LDFLAGS = -lpthread

programs = queue_test
all: $(programs)

test-WaitFree: queue_test.c
	$(CC) $(CFLAGS) -DWaitFree $^ -o $@ $(LDFLAGS)

test-LockFree: queue_test.c
	$(CC) $(CFLAGS) -DLockFree $^ -o $@ $(LDFLAGS)

test-LockBased: queue_test.c
	$(CC) $(CFLAGS) -DLockBased $^ -o $@ $(LDFLAGS)


%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o
	-rm -f $(programs)
