CC=gcc
CFLAGS= -O2 -std=c99 -ffixed-x14 -ffixed-x15
INC= -I./ -Iafcontext/ -I../workloads/

LIB_OBJ=rr_dram_miss_handler.o threads.o jump.o make.o rr_scheduler.o pr_dram_miss_handler.o pr_scheduler.o
TESTS_OBJ=api_test.o test_afcontext.o handler_test.o pr_handler_test.o
TARGETS=libafthread.a api_test test_afcontext handler_test get_cpu_cycle_test pr_handler_test

all: $(TARGETS)

rr_dram_miss_handler.o: rr_dram_miss_handler.S
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

threads.o: threads.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

jump.o: afcontext/jump_arm64_aapcs_elf_gas.S
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

make.o: afcontext/make_arm64_aapcs_elf_gas.S
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

rr_scheduler.o: rr_scheduler.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

pr_dram_miss_handler.o: pr_dram_miss_handler.S
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

pr_scheduler.o: pr_scheduler.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

api_test.o: api_test.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

handler_test.o: handler_test.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

pr_handler_test.o: pr_handler_test.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

test_afcontext.o: afcontext/test_afcontext.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

libafthread.a: $(LIB_OBJ)
	ar rcs $@ $^

api_test: api_test.o libafthread.a
	$(CC) $^ $(CFLAGS) -o $@

handler_test: handler_test.o libafthread.a
	$(CC) $^ $(CFLAGS) -o $@

pr_handler_test: pr_handler_test.o libafthread.a
	$(CC) $^ $(CFLAGS) -o $@

get_cpu_cycle_test: get_cpu_cycle_test.c
	$(CC) $^ $(CFLAGS) $(INC) -o $@

test_afcontext: test_afcontext.o jump.o make.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(LIB_OBJ) $(TESTS_OBJ) $(TARGETS)