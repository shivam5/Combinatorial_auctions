// Random restarts are uniform at random
// 10000 inner iterations
// Probability based greedy

#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<limits.h>
#include<time.h>
#include<vector>

using namespace std;

// Global variables
char *inputfile;
clock_t start_time;

// Defining the problem
struct problem{
	int wallclocktime, noofcoalblocks, noofbids, noofcompanies, startingindexofcompany;
	vector <int> *bids;
}prob;

// Defining the state
struct state{
	int* coalblocks;
	int* companies;
	int* bids;
}current, neighbour;

// Preprocessing the text file and defining the problem accordingly
void preprocess (){
	FILE *file = fopen( inputfile, "r" );

	if (file ==0){
		printf("File couldn't be opened\n");
		return;
	}

	int i, j, k, t, cid, ncid, bidid=0, nbid, bidvalue, block;
	bool first = true;

	fscanf(file, "%d\n", &prob.wallclocktime);
	fscanf(file, "%d\n", &prob.noofcoalblocks);
	fscanf(file, "%d\n", &prob.noofbids);
	fscanf(file, "%d\n", &prob.noofcompanies);

 	// Allocating space for current node
	current.coalblocks= (int*) calloc(prob.noofcoalblocks, sizeof(int));
	current.companies = (int* )calloc(prob.noofcompanies, sizeof(int));
	current.bids = (int* )calloc(prob.noofbids, sizeof(int));

	// Allocating space for neighbour node
	neighbour.coalblocks= (int*) calloc(prob.noofcoalblocks, sizeof(int));
	neighbour.companies = (int* )calloc(prob.noofcompanies, sizeof(int));
	neighbour.bids = (int*) calloc(prob.noofbids, sizeof(int));

	prob.bids = (vector <int> *) calloc(prob.noofbids, sizeof(vector <int>));
	
	for (i=0; i<prob.noofcompanies; i++){
		fscanf(file, "%d %d\n", &cid, &ncid);
		if (first){
			prob.startingindexofcompany = cid;
			first=false;
		}
	
		for (j=0; j<ncid; j++){
			vector <int> bid;
			fscanf(file, "%d %d %d ", &cid, &nbid, &bidvalue);
			bid.push_back(cid);
			bid.push_back(nbid);
			bid.push_back(bidvalue);

			for (k=0; k<nbid; k++){
				fscanf(file, "%d ", &block);
				bid.push_back(block);
			}

			prob.bids[bidid] = bid;
			bid.clear();
			fscanf(file, "\n");
			bidid++;
		}
	}

	fclose(file);
}

// Copy state to copy state from one varible to another
void copystate (struct state X, struct state Y){
	int i;
	for (i=0; i<prob.noofcoalblocks; i++)
		X.coalblocks[i]=Y.coalblocks[i];

	for (i=0; i<prob.noofcompanies; i++)
		X.companies[i]=Y.companies[i];

	for (i=0; i<prob.noofbids; i++)
		X.bids[i]=Y.bids[i];	
}

// Check if a bid conflicts with a state
bool conflict (vector <int> V, struct state S){
	// Check company conflict
	int cid = V[0];
	if (S.companies[cid-prob.startingindexofcompany] != 0)
		return true;

	// Check coalblocks conflicts
	for (int i=3; i<V.size(); i++){
		if (S.coalblocks[V[i]] != 0 )
			return true;
	}

	return false;
}

// Generate a array with number 1 to n in random order
int* generate_random_array (int n){
	int* random_array = (int*) calloc(prob.noofbids, sizeof(int));
	int r, temp, i;

	for(i = 0; i < n; i++)
	    random_array[i] = i;

	for(i = 0; i < (n-1); i++){
    	r = rand()%(n-i-1);
    	temp = random_array[n-i-1];
    	random_array[n-i-1] = random_array[r];
    	random_array[r] = temp;
	}

	return random_array;
}

// Generating successors
void successor (struct state X, int randombid=-1, int valtype=0) {

	int i, cid;
	vector <int> bid;

	// If valtype is not 0, it greedily finds best solution, value = revenue
	if (valtype!=0){
		int maxi =0;

		int maxvalue = INT_MIN;
		int value;

		for (i=0; i<prob.noofbids; i++){
			bid = prob.bids[i];
			value = bid[2];

			if (value > maxvalue){
				int maxi = i;
				cid = bid[0];
				copystate(neighbour, X);
				for (int k=3; k<bid.size(); k++)
					neighbour.coalblocks[bid[k]] = 1;
				maxvalue = value;
				neighbour.companies[cid-prob.startingindexofcompany] = 1;
				neighbour.bids[maxi] = bid[2];
			}
		}

		return;
	}

	else{
		// Valtype is 0, and randombid=-1, finds greedy based on value = bidvalue/coalblocks
		if (randombid == -1){
			float maxvalue = FLT_MIN;
			float value;

			for (i=0; i<prob.noofbids; i++){
				bid = prob.bids[i];
				value = (float) bid[2]/bid[1];

				if (value > maxvalue && !conflict(bid, X) ){
					int maxi = i;
					cid = bid[0];
					copystate(neighbour, X);
					for (int i=3; i<bid.size(); i++){
						neighbour.coalblocks[bid[i]] = 1;
					}
					maxvalue = value;
					neighbour.companies[cid-prob.startingindexofcompany] = 1;
					neighbour.bids[maxi] = bid[2];

				}

			}
			return;
		}

		else {
			// Get randombid and check for conflict
			bid = prob.bids[randombid];
			cid = bid[0];

			if (!conflict(bid, X)){
				copystate(neighbour, X);

				for (int i=3; i<bid.size(); i++){
					neighbour.coalblocks[bid[i]] = 1;
				}

				neighbour.companies[cid-prob.startingindexofcompany] = 1;
				neighbour.bids[randombid] = bid[2];

			}

		}
	}
}

// Checks the revenue of a state
int revenue (struct state X){
	int total = 0;
	int i;
	for (i=0; i<prob.noofbids; i++){
		if(X.bids[i]!=-1)
			total+=X.bids[i];
	}
	return total;
}

// Initializes state in current
void initialstate(struct problem X){
	int i;
	for (i=0; i<X.noofcoalblocks; i++)
		current.coalblocks[i]=0;

	for (i=0; i<X.noofcompanies; i++)
		current.companies[i]=0;

	for (i=0; i<X.noofbids; i++)
		current.bids[i]=-1;

};

// Hillclimbing algorithm
void hillclimbing(struct problem X){

	int i=0, wp=0.3;
	float choose;

	struct state maxstate;
	int maxvalue=INT_MIN;

	maxstate.coalblocks= (int*) calloc(prob.noofcoalblocks, sizeof(int));
	maxstate.companies = (int* )calloc(prob.noofcompanies, sizeof(int));
	maxstate.bids = (int*) calloc(prob.noofbids, sizeof(int));

	clock_t current_time;
	current_time = clock();
	current_time = (current_time - start_time) / CLOCKS_PER_SEC;
	
	int r;
	bool once = true;

	float tm = (float) prob.wallclocktime*59;

	int k=0;
	int *random_array1 = generate_random_array(prob.noofbids);

	while (current_time < tm){
		current_time = clock();
		current_time = (current_time - start_time) / CLOCKS_PER_SEC;

		initialstate(X);
		
		if (once){
			initialstate(X);
			successor(current, -1, -1);
			copystate(maxstate, neighbour);
			maxvalue = revenue(maxstate);
			once=false;
		}

		else if (k<prob.noofbids){
			r = random_array1[k++];
			successor(current, r);		
			copystate(current, neighbour);
		}

		int j=0;

		for(int i=0; i<5000; i++){
			choose = ((double) rand() / (RAND_MAX));

			if (choose < wp){
				r = rand()%prob.noofbids;
				successor(current, r);
				copystate(current, neighbour);
			}
			else{
				successor(current);
				copystate(current, neighbour);
			}
		}

		if (revenue(current)>maxvalue){
			copystate(maxstate, current);
			maxvalue = revenue(maxstate);
		}


	}


	copystate(current, maxstate);

}

int main(int argc, char *argv[]){

	start_time = clock();

	if (argc!=3){
		printf("The syntax is programname <input file> <output file> \n");
	}
	else{
		inputfile = argv[1];

		srand(time(NULL));
		preprocess();
		hillclimbing(prob);

		FILE *file1 = fopen (argv[2], "w");
		fprintf(file1, "%d ", revenue(current));
		for (int i=0; i<prob.noofbids; i++){
			if(current.bids[i]!=-1)
				fprintf(file1, "%d ", i);
		}
		fclose(file1);


	}

return 0;
}