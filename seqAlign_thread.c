/*
* seqAlign.c
*
*  Created on: Jan 18, 2014
*      Author: John
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define GAP_PENALTY 10

int similarity[6][6] = { { 16, 0, 0, 0, 0, 0 }, { 0, 20, 0, 0, 0, 0 }, { 0, 0,
		20, 0, 0, 0 }, { 0, 0, 0, 20, 0, 12 }, { 0, 0, 0, 0, 20, 0 }, { 0, 0, 0,
		12, 0, 20 } };

//struct for passing data
struct threadInfo {
	int id;
	int counter;
};

//struct for linked list
struct node {
	int index;
	struct node* next;
};

struct th_node {
	int index;
	struct th_node* th_next;
};

//the actual head nodes
struct node* head = NULL;
struct th_node* th_head = NULL;

long** dpMatrix;

char* seq1 = NULL;
char* seq2 = NULL;

char* filename;

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


struct threadInfo f(struct threadInfo data){

	int id = data.id;     	/* data received by thread */
	int counter = data.counter;

	printf("head %d\n", getPosNode(&head, id));
	printf("th_head %d\n", getPosTh(&th_head, id));

	dpMatrix[id][counter] = computeSimilarity(id, counter, seq1, seq2);

	//If all work is done 
	//Then cancel the last thread and return the matrix
	if(counter == strlen(seq1)){
	 	
	}//if
	else{
		increment(id, counter);

   		struct threadInfo info; 
    	info.id = id;
    	info.counter = counter + 1;

		return f(info);
	}//else
}//f

/*	
	Compute the values for the DPM 
	Appears to use the Needleman–Wunsch algorithm for calculation
*/
void doWork(void* threadInfo) {

	struct threadInfo* info = (struct threadInfo*) threadInfo;

	int id = info -> id;     	/* data received by thread */
	int counter = info -> counter;
	free(info);

	struct threadInfo data;
	data.id = id;
	data.counter = counter;

	struct threadInfo y = f(data);

	//if the matrix is done 
	//then stop the program
	if (id == strlen(seq2)){
	allDone(); // =)
	}
}

void increment(int id, int counter){

	incVal(&head, id);
	counter++; //increment the counter

	//if a new thread can be made
	if (counter == 2 && id < strlen(seq1)){

		    printf("thread %d Created!\n", id+1);
		
		 int        rc;         		/* return value                           */
	     pthread_t  thread_id = id + 1;     	/* thread's ID (just an integer)          */

		 struct threadInfo *info = malloc(sizeof(struct threadInfo)); 
   		 info -> id = id + 1;
    	 info -> counter = 1;

	    /* create a new thread that will execute 'PrintHello' */
	     rc = pthread_create(&thread_id, NULL, doWork, info);  
	     addValNode(&head, 1);
	     addValTh(&th_head, thread_id);


	    /* could not create thread */
	    if(rc){
	        printf("\n ERROR: return code from pthread_create is %d \n", rc);
	        exit(1);
	    }
	}//if

	//If parent thread is still working on the data above
	//Then lock itself
	if(counter >= getPosNode(&head, counter+1) && (id != 1)){
	}//if

	//If child thread is locked and can be doing work
	//Then unlock child
	if(counter >= getPosNode(&head, counter-1) && id != strlen(seq1)){
		
	}//if

	//If the count is out done with all of the columns 
	//Then cancel the thread
	if(counter > strlen(seq2)){
	 	pthread_cancel(pthread_self());
	}//if
}//increment


//method to push an element to the linked list
void addValNode(struct node** head, int val) {
	//allocate memory
	struct node* n_node = (struct node*) malloc(sizeof(struct node));

	n_node->index = val;
	n_node->next = (*head);
	(*head) = n_node;
}

void addValTh(struct node** th_head, int val) {
	//allocate memory
	struct node* n_node = (struct node*) malloc(sizeof(struct node));

	n_node->index = val;
	n_node->next = (*th_head);
	(*th_head) = n_node;
}

//returns the element at a certain position on a linked list
int getPosNode(struct node* head, int x){
    struct node* curr = head;
    int count = 0; 
    while  (curr != NULL)
    {
       if (count == x){
          	return (curr->index);
       }
       count++;
       curr = curr->next;
    }
    assert(0);              
}

int getPosTh(struct th_node* head, int x){
    struct node* curr = head;
    int count = 0; 
    while  (curr != NULL){
       if (count == x){
          	return (curr->index);
       }
       count++;
       curr = curr->next;
    }
    assert(0);              
}
	/*if(head == NULL){
		return NULL;
	}
	while(--pos){
		if(head->next){
			head = head->next;
		} else {
			return NULL;
		}
	}
	return (head->index);*/

//increments a value in the linked list by one
void incVal(struct node* head, int pos){
	int temp;

	//check for null, if it is make index 1
	if((head->index) != NULL){
		temp = head->index;
		temp++;
		head->index = temp;
	} else {
		temp = 1;
		head->index = temp;
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

void allDone(){

	printf("Writing to file (may take some time)\n");
	
	outputMatrix(filename, strlen(seq1), strlen(seq2));

	freeMatrixMemory(strlen(seq1), strlen(seq2));
	free(seq1);
	free(seq2);
	exit(EXIT_SUCCESS);
}//allDone

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
	filename = argv[2];

	getline(&seq1, &len, fp);
	trim(seq1); // getline() includes newline

	getline(&seq2, &len, fp);
	trim(seq2); // getline() includes newline

	initMatrix(strlen(seq1), strlen(seq2));

	generateGaps();

	//creates the first unique thread and starts the computing process
	int        rc;         	/* return value                           */
    pthread_t  thread_id;     	/* thread's ID (just an integer)          */

    struct threadInfo *info = malloc(sizeof(struct threadInfo)); 
    info -> id = 1;
    info -> counter = 1;

	

    rc = pthread_create(&thread_id, NULL, doWork, info); 

	printf("The thread id is  %d\n", thread_id);

    addValNode(&head, 1);
    addValTh(&th_head, thread_id);

    printf("Head: %d\n", getPosHead(&head,1));
    printf("Th_Head: %d\n", getPosTh(&th_head,1));

    /* could not create thread */
    if(rc){
        printf("\n ERROR: return code from pthread_create is %d \n", rc);
        exit(1);
    }
    pthread_exit(NULL);
}

		