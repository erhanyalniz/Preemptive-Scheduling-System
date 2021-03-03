//Erhan Yalniz   -   150117905

#include <stdio.h>
#include <stdlib.h> // malloc()
#include <math.h> // exp(), pow()

// Will be later used to delete a element in heap by making key a maximum value.
#define INTMAX 2147483647

// This is structure of a process. Will be used in binomial heap as element. 
typedef struct{
    int pid; // Process id.
    double e, t_arr, priorty; // execution time, arrival time, priorty of process.
}node;

node *root; // This will hold our binomial heap as a array(pointer).
// size will hold the current size of binomial heap (elements in binomial heap), 
// capacity will hold capacity of binomial heap (maximum number of elements that binomial heap can hold),
// WT will hold each processes waiting times.
int size, capacity, *WT, number_of_processes;
double e_max; // This will hold longest execution time.
double **input_list; // This will hold process ids, execution times, arrival times read from the input file.

// Read input file "filename" and fill information to "input_list".
int read_input_list(char filename[]);

// Initialize binomial heap (the "root" array) with capacity of "cap".
void initialize(int cap);

// Just a simple swap function for process structure type items "a" and "b".
// This is later used in basic functions of binomial heap.
void swap(node *a, node *b);

// Get index of process with process id "pid" from "input_list".
int indexOfPID(int pid);

// Get parent of node with index "i" from binomial heap "root" and return index of it.
int parent(int i);

// Get index of left child of node at index "i" from binomial heap "root". 
int left(int i);
  
// Get index of right child of node at index "i" from binomial heap "root".
int right(int i);

// Reorder binomial heap "root" in order of priorty values.
void heapify(int i);

// Extract (remove and return) the first element from binomial heap "root" which is the minimum element.
node extract();
  
// Decreases priorty value at index "i" in binomial heap "root" to "new_val".
void decrease(int i, double new_val); 
  
// Returns the minimum key (key at root) from min heap.
node minimum();

// Check if execution time "e" is already in "input_list". If it is return 1 otherwise 0.
int execution_time_in_process_list(double e);

// Formula of c. "first_insertion" is 1 when element is first inserted into binomial heap "root" otherwise 0.
double c(double e, int first_insertion);

// Formula of f. Used to calculate new priorty value. "first_insertion" is 1 when element is first inserted into binomial heap "root" otherwise 0.
double f(double e, double t_arr, int first_insertion);

// Deletes a element stored at index "i" in binomial heap "root".
void delete(int i); 

// Inserts a element into binomial heap "root". Parameters pid, e, t_arr will be used to create element and insert it into binomail heap. "first_insertion" parameter will be used to calculate priorty of inserted process.
void insert(int pid, double e, double t_arr, int first_insertion);

int main(){
    // "t" will hold the current time, "q" will hold quantum (time slice), "i" is the index of current process that is waiting to be inserted into binomial heap "root".
    // "processes_done" will hold number of processes that are done.
    int t, q = 3, i = 0, j, processes_done = 0;
    // This node will hold the process that is currently running on microprocessor.
    node running;
    // Read input file and save number of processes read to "number_of_processes".
    number_of_processes = read_input_list("input.txt");
    // Allocate space for Waiting Time of each process.
    WT = (int *) malloc(sizeof(int) * number_of_processes);
    // Initialize all of waiting times to 0.
    for(j = 0;j < number_of_processes;j++){
        WT[j] = 0;
    }
    // Initialize binomial heap "root" with capacity "capacity".
    initialize(capacity);
    // Time loop starts here. Loop continues unless all the processes are done and increments by quantum (time slice) number.
    for(t = 0;processes_done != number_of_processes;t+=q){
        // If index "i" is less than total number of processes to be executed this means there is still new processes to insert and run on processor.
        // Next we have to check if arrival time of process that is about to be inserted is on time.
        if(i < number_of_processes && input_list[i][2] <= t){
            // If all the conditions apply process should be inserted as first insertion to binomial heap "root".
            insert(input_list[i][0], input_list[i][1], input_list[i][2], 1);
            // Increment the index of processes to be pulled from "input_list".
            i++;
        }
        // Get the process with minimum value of priorty. Which is the first element of "root" array.
        running = minimum();
        // Print information about current execution.
        printf("PID: %d  at t = %d with priorty = %lf\n", running.pid, t, running.priorty);
        // If execution time of running process is longer than quantum number prempt the running process.
        if(running.e > q){
            // Recalculate the priorty of process to be empted.
            running.priorty = f(running.e - q, running.t_arr, 0);
            // Decrease the remaining runtime of process by quantum number.
            running.e-=q;
            // Delete old process from binomial heap as pid doesn't change use it as a key to find old process in binomial heap.
            delete(running.pid);
            // Insert new process as preempted and not first insertion.
            insert(running.pid, running.e, running.t_arr, 0);
        }
        // If execution time fits quantum number execute and finish process in one go.
        else{
            // Increment time by execution time as it takes exactly that amount of time.
            // But dont forget to decrease one quantum number as we already gonna increment it by quantum number at the end of for loop.
            t+=running.e - q;
            // Calculate the waiting time by decreasing arrival time and total execution time from time at which process ended.
            // Dont forget time we add "q" to "t" to get actual time process ended.
            // And by adding arrival time and total execution time we get normally expected time for process to finish.
            // By subtracting this from actual ending time of process we get Waiting Time.
            WT[indexOfPID(running.pid)] += t + q - running.t_arr - input_list[indexOfPID(running.pid)][1];
            // End the process by making execution time 0.
            running.e = 0;
            // Extract the process from binomial heap.
            extract();
            // Increment total number of processes that are done.
            processes_done++;
        }
    }
    // Print each one of the waiting times.
    for(i = 0;i < number_of_processes; i++){
        printf("P%d\t%d\n", i+1, WT[i]);
    }
    // Calculate Average Waiting Time by summing up and dividing by number of processes.
    double AWT = 0;
    for(i = 0;i < number_of_processes; i++){
        AWT+=WT[i];
    }
    AWT/=number_of_processes;
    // Print the Average Waiting Time.
    printf("AWT = %lf\n", AWT);
    return 0;
}

// Read input file "filename" and fill information to "input_list".
int read_input_list(char filename[]){
    // Open file as read.
    FILE* fptr = fopen(filename, "r");
    // Temporary variable that will hold process id for current line read.
    int pid;
    // Temporary variables that will hold execution time and arrival time for current line read.
    double e, t_arr;
    int i = 0;
    // Allocate space for "input_list" array.
    input_list = (double **) malloc(sizeof(double *));
    // Scan line for current process. If input file ended fscanf will not return 3 so the loop will end.
    while(fscanf(fptr, "P%d %lf %lf\n", &pid, &e, &t_arr) == 3){
        // Allocate space for current row in "input_list" array.
        input_list = (double **) realloc(input_list, sizeof(double *) * (i + 1));
        // Allocate space for current columns of current row.
        input_list[i] = (double *) malloc(sizeof(double) * 3);
        // Store process id in first column of the current row.
        input_list[i][0] = pid;
        // Store execution time in second column of the current row.
        input_list[i][1] = e;
        // Store third in first column of the current row.
        input_list[i][2] = t_arr;
        // If current execution time is bigger than maximum execution time reassign maximum execution time.
        if(e > e_max)
            e_max = e;
        // Increment index.
        i++;
    }
    // Calculate capacity (maximum number elements in binomal heap) by using property of binomial heap.
    // Binomial heap always have 2^k elements.
    // By this property we can calculate capacity of our binomial heap, taking base 2 logarithm of number of processes,
    // and rounding that value to ceiling, taking power of it by 2, we get nearest multiple of 2 to number of processes "i". 
    capacity = (int) pow(2, ceil(log(i)/log(2)));
    // Close file.
    fclose(fptr);
    // Return total number of processes read.
    return i;
}

// Initialize binomial heap (the "root" array) with capacity of "cap".
void initialize(int cap){
    // Set initial size  of binomial heap to 0.
    size = 0;
    // Allocate space for binomail heap.
    root = (node *) malloc(sizeof(node)*cap);
}

// Just a simple swap function for process structure type items "a" and "b".
// This is later used in basic functions of binomial heap.
void swap(node *a, node *b){
    int temp1;
    double temp2;
    
    temp1 = a->pid;
    a->pid = b->pid;
    b->pid = temp1;

    temp2 = a->e;
    a->e = b->e;
    b->e = temp2;
    
    temp2 = a->t_arr;
    a->t_arr = b->t_arr;
    b->t_arr = temp2;
    
    temp2 = a->priorty;
    a->priorty = b->priorty;
    b->priorty = temp2;
}

// Get index of process with process id "pid" from "input_list".
int indexOfPID(int pid){
    int i;
    for(i = 0; i < number_of_processes; i++){
        if(input_list[i][0] == pid){
            // If process id found return index of it.
            return i;
        }
    }
    // If process id cannot be found inside "input_list" return -1.
    return -1;
}

// Get parent of node with index "i" from binomial heap "root" and return index of it.
int parent(int i){
    // Get the nearest floor multiple of 2 which is parent of node at "i".
    return (i-1)/2;
}

// Get index of left child of node at index "i" from binomial heap "root". 
int left(int i){
    return (2*i + 1);
}

// Get index of right child of node at index "i" from binomial heap "root".
int right(int i){
    return (2*i + 2);
}

// Reorder binomial heap "root" in order of priorty values.
void heapify(int i){ 
    // Get the left child node of current element "i".
    int l = left(i);
    // Get the right child node of current element "i".
    int r = right(i);
    // Assume parent node to be smallest.
    int smallest = i;
    // If left element does exist index of it must be less then current size of heap.
    if(l < size && root[l].priorty < root[i].priorty)
        smallest = l; // If priorty of the left child is smaller than current smallest element then reassign.
    if(r < size && root[r].priorty < root[smallest].priorty)
        smallest = r; // If priorty of the right child is smaller than current smallest element then reassign.
    if(smallest != i){
        // If parent is not the node with smallest process id swap it with the smallest one.
        swap(&root[i], &root[smallest]);
        // Apply this process recursively to new smallest element.
        heapify(smallest);
    }
} 

// Extract (remove and return) the first element from binomial heap "root" which is the minimum element.
node extract(){
    // If the size is 0 there is no element to extract return error.
    if(size == 0){
        node error = {-1,0,0,INTMAX};
        return error;
    }
    // If this is the last element of binomial heap "root" retun first element of array and decrease size.
    if(size == 1){
        size--;
        return root[0];
    }
  
    // Store the minimum value which is first element of array, and remove it from heap.
    node res;
    res.pid = root[0].pid;
    res.e = root[0].e;
    res.t_arr = root[0].t_arr;
    res.priorty = root[0].priorty;
    
    // Make the last element of array which is the last deepest element of heap.
    root[0].pid = root[size - 1].pid;
    root[0].e = root[size - 1].e;
    root[0].t_arr = root[size - 1].t_arr;
    root[0].priorty = root[size - 1].priorty;

    // Decrease size.
    size--;
    // Reorder the heap starting from root.
    heapify(0);
    // Return extracted element.
    return res; 
}

// Decreases priorty value at index "i" in binomial heap "root" to "new_val".
void decrease(int i, double new_val){
    // Assign "new_val" to priorty of current node with index "i".
    root[i].priorty = new_val;
    // Reorder all elements if this is not the first node.
    while (i != 0 && root[parent(i)].priorty > root[i].priorty){ 
        // Swap parent of current element and itself while priorty of the parent is greater than current element.
        swap(&root[i], &root[parent(i)]);
        // Change index to parent element.
        i = parent(i);
    } 
}

// Returns the minimum key (key at root) from min heap.
node minimum(){
    // Root element is always the element with minimum priorty value.
    return root[0];
}

// Check if execution time "e" is already in "input_list". If it is return 1 otherwise 0.
int execution_time_in_process_list(double e){
    int i, count = 0;
    // Iterate through entire "input_list".
    for(i = 0;i < size;i++){
        // If current element in "input_list" execution time is same as e. Increase count.
        if(input_list[i][1] == e)
            count++;
        // If the count is more than one there is duplicate of same execution time with different process id.
        if(count > 1)
            return 1;
    }
    return 0;
}

// Formula of c. "first_insertion" is 1 when element is first inserted into binomial heap "root" otherwise 0.
double c(double e, int first_insertion){
    // If it is first insertion c(e) is just 1.
    if(first_insertion){
        return 1;
    }
    // Otherwise apply the formula: 1/exp(-1* (2*e_i/(3 * e_max) ^ 3).
    else{
        return  (1.0 / exp(-pow(2 * e / (3 * e_max), 3)));
    }
}

// Formula of f. Used to calculate new priorty value. "first_insertion" is 1 when element is first inserted into binomial heap "root" otherwise 0.
double f(double e, double t_arr, int first_insertion){
    // If the execution time is not in process list "input_list" f(e, t_arr) = c(e_i) * e_i
    if(!execution_time_in_process_list(e)){
        return c(e, first_insertion)*e;
    }
    // Otherwise f(e, t_arr) = t_arr
    else{
        return t_arr;
    }
}

// Deletes a element stored at index "i" in binomial heap "root".
void delete(int i) 
{
    int j;
    // Try to find element with pid = "i".
    for(j = 0;j < size;j++){
        if(root[j].pid == i)
            break;
    }
    // Decrease found elements priorty value to maximum integer.
    decrease(i, INTMAX); 
    // Extract it from heap.
    extract(); 
}

// Inserts a element into binomial heap "root". Parameters pid, e, t_arr will be used to create element and insert it into binomail heap. "first_insertion" parameter will be used to calculate priorty of inserted process.
void insert(int pid, double e, double t_arr, int first_insertion){
    int i;
    // If size is equal to capacity there is no space left for new element to insert.
    if (size == capacity) 
    {
        printf("Overflow: Could not insert.\n");
        return; 
    }
    // First insert the new element at the end.

    // Increment size.
    size++;
    // Get the index of element to insert.
    i = size - 1;
    // Insert element.
    root[i].pid = pid;
    root[i].e = e;
    // Calculate priorty value.
    root[i].priorty = f(e, t_arr, first_insertion);
    root[i].t_arr = t_arr;
    // Fix the min heap property if it is violated.
    while (i != 0 && root[parent(i)].priorty > root[i].priorty) 
    { 
       // Swap if parent has greater priorty than current element wşth index "i".
       swap(&root[i], &root[parent(i)]); 
       // Repeat the same process by changing index to parent.
       i = parent(i); 
    }
}
