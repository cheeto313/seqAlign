CC = gcc
PTHREAD_LIBS = -lpthread
PROGRAM_SEQ = seqAlign
PROGRAM_THREAD = seqAlign_thread
PROGRAM_GEN = genSequence
CFLAGS = -std=c99
THREADS = 8
SOLUTION_DIR = solution
DATA_DIR = data
RESULTS_DIR = result

%.o: ${SRC}/%.c
	$(CC) -c -o ${OBJDIR}/$@ $< $(CFLAGS)

all: ${PROGRAM_SEQ} ${PROGRAM_GEN} ${PROGRAM_THREAD}
	
clean:
	rm -f *.o *.stackdump ${PROGRAM_SEQ} ${PROGRAM_GEN} ${PROGRAM_THREAD}	
	
seq: ${PROGRAM_SEQ}.o
	${CC} -o ${PROGRAM_SEQ} ${PROGRAM_SEQ}.o

par: ${PROGRAM_THREAD}.o
	${CC} -o ${PROGRAM_THREAD} ${PROGRAM_THREAD}.o
	
${PROGRAM_GEN}: ${PROGRAM_GEN}.o
	${CC} -o ${PROGRAM_GEN} ${PROGRAM_GEN}.o

solutions: ${PROGRAM_SEQ}
	mkdir -p ${SOLUTION_DIR}
	./${PROGRAM_SEQ} ${DATA_DIR}/length3.seq ${SOLUTION_DIR}/length3.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length50.seq ${SOLUTION_DIR}/length50.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length100.seq ${SOLUTION_DIR}/length100.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length500.seq ${SOLUTION_DIR}/length500.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length1000.seq ${SOLUTION_DIR}/length1000.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length2000.seq ${SOLUTION_DIR}/length2000.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length3000.seq ${SOLUTION_DIR}/length3000.solution
	./${PROGRAM_SEQ} ${DATA_DIR}/length4000.seq ${SOLUTION_DIR}/length4000.solution

# Testing 	
test3: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD}.exe ${DATA_DIR}/length3.seq ${THREADS} ${RESULTS_DIR}/length3.result
	diff ${RESULTS_DIR}/length3.result ${SOLUTION_DIR}/length3.solution

test50: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD} ${DATA_DIR}/length50.seq ${THREADS} ${RESULTS_DIR}/length50.result
	diff ${RESULTS_DIR}/length50.result ${SOLUTION_DIR}/length50.solution

test100: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD} ${DATA_DIR}/length100.seq ${THREADS} ${RESULTS_DIR}/length100.result
	diff ${RESULTS_DIR}/length100.result ${SOLUTION_DIR}/length100.solution

test500: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD} ${DATA_DIR}/length500.seq ${THREADS} ${RESULTS_DIR}/length500.result
	diff ${RESULTS_DIR}/length500.result ${SOLUTION_DIR}/length500.solution
	
test1000: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD} ${DATA_DIR}/length1000.seq ${THREADS} ${RESULTS_DIR}/length1000.result
	diff ${RESULTS_DIR}/length1000.result ${SOLUTION_DIR}/length1000.solution
		
test2000: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD} ${DATA_DIR}/length2000.seq ${THREADS} ${RESULTS_DIR}/length2000.result
	diff ${RESULTS_DIR}/length2000.result ${SOLUTION_DIR}/length2000.solution

test4000: ${PROGRAM_THREAD}
	time ./${PROGRAM_THREAD} ${DATA_DIR}/length4000.seq ${THREADS} ${RESULTS_DIR}/length4000.result
	diff ${RESULTS_DIR}/length4000.result ${SOLUTION_DIR}/length4000.solution
	