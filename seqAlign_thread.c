/*
 * seqAlign.c
 *
 *  Created on: Jan 18, 2014
 *      Author: John
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GAP_PENALTY 10
#define BLOCKSIZE 1000

int similarity[6][6] = { { 16, 0, 0, 0, 0, 0 }, { 0, 20, 0, 0, 0, 0 }, { 0, 0,
		20, 0, 0, 0 }, { 0, 0, 0, 20, 0, 12 }, { 0, 0, 0, 0, 20, 0 }, { 0, 0, 0,
		12, 0, 20 } };

typedef struct workItem {
	int fromX;
	int fromY;
	int toX;
	int toY;
	int x;
	int y;
} workItem_t;

typedef struct node {
	workItem_t item;
	struct node *next;
} workQueue_t;

long** dpMatrix;

char* seq1 = NULL;
char* seq2 = NULL;

int numBlocks_x, numBlocks_y;

workQueue_t* workQueue;

workItem_t** work;

int** queued;

/* Find the max of three numbers */
int max(int a, int b, int c) {
	int x = (a > b) ? a : b;
	int max = (x > c) ? x : c;
	return max;
}

/* Trim whitespace from end of line */
void trim(char* str) {
	char* end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;

	// Write new null terminator
	*(end + 1) = 0;
}

/* Translate a character to an index in the similarity matrix */
int charIndex(char c) {
	c = (char) toupper(c);
	switch (c) {
	case 'A':
		return 0;
	case 'D':
		return 1;
	case 'K':
		return 2;
	case 'L':
		return 3;
	case 'T':
		return 4;
	case 'V':
		return 5;
	default:
		return -1;
	}
}

/* Compute the similarity at a DPM location */
int computeSimilarity(int x, int y, char* seq1, char* seq2) {
	if (x == 0) {
		if (y == 0)
			return 0;
		return dpMatrix[x][y - 1] - GAP_PENALTY;
	}

	if (y == 0) {
		return dpMatrix[x - 1][y] - GAP_PENALTY;
	}

	int left = dpMatrix[x - 1][y] - GAP_PENALTY;
	int above = dpMatrix[x][y - 1] - GAP_PENALTY;

	char a = seq1[x - 1];
	char b = seq2[y - 1];
	int diagonal = dpMatrix[x - 1][y - 1]
			+ similarity[charIndex(a)][charIndex(b)];

	return max(left, above, diagonal);
}

/* Write the DPM to a file */
void outputMatrix(char* filename, int width, int height) {

	FILE* file = fopen(filename, "w");
	if (file == NULL ) {
		fprintf(stderr, "Unable to open %s for writing.", filename);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < width + 1; i++) {
		for (int j = 0; j < height + 1; j++) {
			fprintf(file, "%d\t", dpMatrix[i][j]);
		}
		fprintf(file, "\n");
	}
	fprintf(file, "\n");
	fclose(file);
}

/* Setup the DPM */
void initMatrix(int width, int height) {
	dpMatrix = (long**) calloc(width + 1, sizeof(long*));
	for (int i = 0; i < width + 1; i++) {
		dpMatrix[i] = (long*) calloc(height + 1, sizeof(long));
	}
}

/* Cleanup memory of DPM */
void freeMatrixMemory(int width, int height) {
	for (int i = 0; i < width + 1; i++) {
		free(dpMatrix[i]);
	}
	free(dpMatrix);
}

/* Compute the values for the DPM */
void doWork() {

	// TO BE IMPLEMENTED
}

void compute(char* seq1, char* seq2, int numThreads) {

	// Create work items
	numBlocks_x = (strlen(seq1) + 1) / BLOCKSIZE;
	if (numBlocks_x * BLOCKSIZE < strlen(seq1) + 1)
		numBlocks_x++;

	numBlocks_y = (strlen(seq2) + 1) / BLOCKSIZE;
	if (numBlocks_y * BLOCKSIZE < strlen(seq2) + 1)
		numBlocks_y++;

	work = calloc(numBlocks_x, sizeof(workItem_t*));
	queued = calloc(numBlocks_x, sizeof(int*));
	for (int i = 0; i < numBlocks_x; i++) {
		work[i] = calloc(numBlocks_y, sizeof(workItem_t));
		queued[i] = calloc(numBlocks_y, sizeof(int));
	}

	for (int i = 0; i < numBlocks_x; i++) {
		for (int j = 0; j < numBlocks_y; j++) {
			work[i][j].fromX = i * BLOCKSIZE;
			work[i][j].fromY = j * BLOCKSIZE;
			work[i][j].toX =
					((i + 1) * BLOCKSIZE > strlen(seq1) + 1) ?
							strlen(seq1) + 1 : (i + 1) * BLOCKSIZE;
			work[i][j].toY =
					((j + 1) * BLOCKSIZE > strlen(seq2) + 1) ?
							(strlen(seq2) + 1) : (j + 1) * BLOCKSIZE;
			work[i][j].x = j;
			work[i][j].y = i;
		}
	}

	int workQueueSize = (numBlocks_x > numBlocks_y) ? numBlocks_x : numBlocks_y;

	workQueue = malloc(sizeof(struct node));
	workQueue->item = work[0][0];
	workQueue->next = NULL;
	queued[0][0] = 1;
	doWork();
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		fprintf(stderr,
				"Usage: seqAlign_thread <data file> <# of threads> [output file]\n");
		exit(EXIT_FAILURE);
	}

	FILE * fp;
	size_t len = 0;
	fp = fopen(argv[1], "r");
	if (fp == NULL ) {
		fprintf(stderr, "File not found: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	int threads = atoi(argv[2]);

	getline(&seq1, &len, fp);
	trim(seq1); // getline() includes newline

	getline(&seq2, &len, fp);
	trim(seq2); // getline() includes newline

	initMatrix(strlen(seq1), strlen(seq2));
	compute(seq1, seq2, threads);

	if (argc == 4) {
		printf("Writing to file (may take some time)\n");
		char* filename = argv[3];
		outputMatrix(filename, strlen(seq1), strlen(seq2));
	}

	freeMatrixMemory(strlen(seq1), strlen(seq2));
	free(seq1);
	free(seq2);

	exit(EXIT_SUCCESS);
}

