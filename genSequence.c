#include <stdlib.h>
#include <stdio.h>

/**
 * This program generates two random sequences based on the size of string (for reproducability)
 */

char chars[6] ={'A','D','K','L','T','V'};

int main(int argc, char* argv[]) {

	if (argc < 3) {
		fprintf(stderr, "Usage: genSequence <output file> <size>\n");
		exit(EXIT_FAILURE);
	}


	char* fileName = argv[1];
	int seqSize = atoi(argv[2]);

	printf("file: %s\n",fileName);
	printf("size: %d\n",seqSize);

	char seq1[seqSize+1];
	char seq2[seqSize+1];

	srand(seqSize);
	for(int i=0; i<seqSize; i++){
		seq1[i] = chars[rand() % 6];
		seq2[i] = chars[rand() % 6];
	}
	seq1[seqSize] = 0;
	seq2[seqSize] = 0;

	FILE* outputFile = fopen(fileName,"w");
	fprintf(outputFile,"%s\n",seq1);
	fprintf(outputFile,"%s\n",seq2);
	fclose(outputFile);

	exit(EXIT_SUCCESS);
}
