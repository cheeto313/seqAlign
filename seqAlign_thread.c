/*
* seqAlign.c
*
*  Created on: Jan 18, 2014
*      Author: John
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define GAP_PENALTY 10
#define BLOCKSIZE 1000

int similarity[6][6] = { { 16, 0, 0, 0, 0, 0 }, { 0, 20, 0, 0, 0, 0 }, { 0, 0,
		20, 0, 0, 0 }, { 0, 0, 0, 20, 0, 12 }, { 0, 0, 0, 0, 20, 0 }, { 0, 0, 0,
		12, 0, 20 } };

//struct for linked list
struct node {
	int row;
	struct node* next;
};

struct th_node {
	int pid;
	struct th_node* next;
};

//node for the PID tracking
struct node* th_head = NULL;

//the actual node
struct node* head = NULL;

long** dpMatrix;

char* seq1 = NULL;
char* seq2 = NULL;

int numBlocks_x, numBlocks_y;

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

/*	
	Compute the values for the DPM 
	Appears to use the Needleman–Wunsch algorithm for calculation
*/
void doWork(void* data) {

	int my_data = (int)data;     	/* data received by thread */

    pthread_detach(pthread_self());
    printf("Hello from new thread - got %d\n", my_data);
    printf("getPid gives me %d\n", getpid());
    printf("gettid gives me %d\n", gettid());
    printf("th_head at 0 gives me %d\n", getPos(&th_head, 0));
    printf("head at 0 gives me %d\n", getPos(&head, 0));	
    printf("th_head at 1 gives me %d\n", getPos(&th_head, 1));
    printf("head at 1 gives me %d\n", getPos(&head, 1));
    pthread_exit(NULL);			/* terminate the thread */

	dpMatrix[getpid()+1][getPos(&head, getpid()+1)] = computeSimilarity(getpid(), getPos(&head, getpid()), seq1[getpid()], seq2[getPos(&head, getpid())]);

	//If all work is done 
	//Then cancel the last thread and return the matrix
	if((getPos(&head, getpid()) == strlen(seq1)) && (getpid() == strlen(seq2))){
	 	//pthread_cancel(pthread_self());
	 	pthread_exit(NULL);
	}//if

	//increment();
}

void increment(){

	incVal(&head, getPos(&head, getpid()));

	//if a new thread can be made
	if ((getPos(&head, getpid()) == 1) && (getpid() <= strlen(seq1))){
		
		int        rc;         	/* return value                           */
	    pthread_t  thread_id;     	/* thread's ID (just an integer)          */
	    int        t         = 11;  /* data passed to the new thread          */

	    /* create a new thread that will execute 'PrintHello' */
	    rc = pthread_create(&thread_id, NULL, doWork, (void*)t);  
	    if(rc)			/* could not create thread */
	    {
	        printf("\n ERROR: return code from pthread_create is %d \n", rc);
	        exit(1);
	    }

	    printf("\n Created new thread (%d) ... \n", thread_id);

	}//if

	//If parent thread is still working on the data above
	//Then lock itself
	if(getPos(&head, getpid()) >= getPos(&head, getpid()-1) && (getpid() != 0)){
		pthread_mutex_lock(pthread_self());
	}//if

	//If child thread is locked and can be doing work
	//Then unlock child
	if(getPos(&head, getpid()) > getPos(&head, getpid()+1) && (getpid() != strlen(seq1))){
		pthread_mutex_unlock(pthread_getspecific(getpid()+1));
	}//if

	//If the count is out done with all of the columns 
	//Then cancel the thread
	if(getPos(&head, getpid()) > strlen(seq2)){
	 	//pthread_cancel(pthread_self());
	 	pthread_exit(NULL);
	}//if
/*
Linked List Use examples
	Add element
		addVal(&head,4);
	Get Nth element
		int derp = getPos(&head,2); //return element at index 2

	e.g.
	addVal(&head,4, false); //adds it for table list
	addVal(&head,1, true); //adds it for PID list
	addVal(&head,3);
	addVal(&head,0);
	addVal(&head,6);
	Constructs: 
	6->0->3->1->4
*/
}//increment


//method to push an element to the linked list
void addVal(struct node** head, int val) {
	//allocate memory
	struct node* n_node = (struct node*) malloc(sizeof(struct node));

	n_node->row = n_node;
	n_node->next = (*head);
	(*head) = n_node;
}

//returns the element at a certain position on a linked list
int getPos(struct node* head, int pos) {
	
	if(head == NULL){
		return NULL;
	}
	while(--pos){
		if(head->next){
			head = head->next;
		}
		else{
			return NULL;
		}
	}
	return head;
}

//increments a value in the linked list by one
void incVal(struct node* head, int pos){
	int temp;

		//check for null, if it is make row 1
		if((head->row) != NULL){
			temp = head->row;
			temp++;
			head->row = temp;
		} else {
			temp = 1;
			head->row = temp;
		}		
}


void generateGaps(){
	//placing the gaps for the first column
	for (int i = 0; i < strlen(seq1)+1; i++){
		dpMatrix[0][i] = i * (-1 * GAP_PENALTY);
	}//for i 

	//placing the gaps for the first row
	for (int i = 0; i < strlen(seq2)+1; i++){
		dpMatrix[i][0] = i * (-1 * GAP_PENALTY);
	}//for i
}//generateGaps

int main(int argc, char* argv[]) {

	if (argc < 2) {
		fprintf(stderr,
		"Usage: seqAlign_thread <data file> [output file]\n");
	}

	FILE * fp;
	size_t len = 0;
	fp = fopen(argv[1], "r");
	
	if (fp == NULL ) {
		fprintf(stderr, "File not found: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	getline(&seq1, &len, fp);
	trim(seq1); // getline() includes newline

	getline(&seq2, &len, fp);
	trim(seq2); // getline() includes newline

	initMatrix(strlen(seq1), strlen(seq2));

	generateGaps();

	//creates the first unique thread and starts the computing process
	int        rc;         	/* return value                           */
    pthread_t  thread_id = 1;     	/* thread's ID (just an integer)          */
    int        t = 11;  /* data passed to the new thread          */

    /* create a new thread that will execute 'PrintHello' */

    addVal(&head, 1);
	addVal(&th_head, 1);

    rc = pthread_create(&thread_id, NULL, doWork, (void*)t);  

    

    if(rc)			/* could not create thread */
    {
        printf("\n ERROR: return code from pthread_create is %d \n", rc);
        exit(1);
    }

    printf("\n Created new thread (%d) ... \n", thread_id);
    
    pthread_exit(NULL);		/* terminate the thread */

	if (argc == 3) {
		printf("Writing to file (may take some time)\n");
		char* filename = argv[2];
		outputMatrix(filename, strlen(seq1), strlen(seq2));
	}
		freeMatrixMemory(strlen(seq1), strlen(seq2));
		free(seq1);
		free(seq2);
		exit(EXIT_SUCCESS);
}//main


