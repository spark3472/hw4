# README
<h1>Implementing a Userthread Library</h1>
<h2>To Compile and Run</h2>
Library: <br >
make <br  >

Tests: <br >
After compiling and linking the library together <br >
gcc -o test_name test_name.c -L. -luserthread -lpthread (the userthread library must be in the same directory as the test for this line to work) <br >
./test_name

<h2>Introduction</h2>
This userthread library implements the basic functions of a pthread library such as thread_libinit(), thread_libterminate(), thread_create(), thread_join(), and thread_yield(). <br >
thread_libinit() is used to initialize the queue of threads (library). <br >
thread_libterminate() is used to clean and get rid of the threads and the thread queue (library).<br >
thread_create() is used to create a thread and add it to the queue.<br >
thread_join() is used to make sure all of the threads have run to finish.<br >
thread_yield() is used to make the current/calling thread "yield" (be suspended) and run the next runnable thread in the queue.<br >

<h3>Data Structures Implemented</h3>
The primary data structure implemented was a singly linked list to act as the thread queue. It is initialized by thread_libinit() and freed by thread_libterminate(). thread_create() calls add_thread() which acts as a push function and adds a newly created thread to the linked list (thread queue). thread_join() and thread_yield() as well as the schedulers all use the linked list to find the next runnable thread (thread_yield() and the schedulers) or to find a thread by its id to check its state (thread_join). If the SJF or the PRIORITY policy is selected, the linked list is sorted by calling the sort() function by either the shortest estimated run-time (SJF), or the priority number (PRIORITY).

<h3>Schedulers</h3>
There are three schedulers, one for each policy: FIFO, SJF, and PRIORITY. Which scheduler is used is decided in thread_libinit() when the user passes the respective policy number to choose a certain policy. The FIFO policy schedules the threads in the order they arrive. The SJF policy schedules the threads first by a set estimated run-time (half the quanta = 50ms), and then if thread_yield() is used, new estimated times are created every three times the thread is run. The PRIORITY scheduler sorts the list by priority number, with -1 having the highest priority, 0 having the next, and 1 having the lowest priority. Every 9+6+4 = 19 times the scheduler is called, or every 19 * 100ms, priority number -1 will have run 9 times, 0 6 times, and -1 4 times, given that the threads have not run to finish before they could run their respective number of times. Therefore, threads with priority of -1 will run 1.5 more times than threads of priority 0, which will run 1.5 more times than threads of priority 1.

<h3>Timer</h3>
A timer is called every 100 ms which called the scheduler. This timer is initialized in thread_libinit() and does not need to be freed. Because it uses ITIMER_VIRTUAL, it sends a SIGVTALRM every 100ms, which is caught by the signal hander which then calls the scheduler. 


<h2>Features</h2>

<h3>Fully Implemented</h3>
<h4>thread_libinit()</h4>
Initializes the global thread queue as well as starts the timer as well as chooses the correct scheduler for the chosen policy. 
<h4>thread_libterminate()</h4>
Frees each uc_stack.ss_sp for each thread, then frees the thread, then frees the thread queue by calling free(), then frees the file opener by calling fclose().
<h4>thread_create()</h4>
Initializes each feature of the thread such as the arguments, the function, the ucontext, etc. Calls add_thread to push the thread onto the queue. 
<h4>thread_join()</h4>
Runs a never ending while loop that continuously checks whether or not the specified thread id is finished by checking its state.
<h4>thread_yield()</h4>
Swaps to another context, thereby suspending the current/calling context. The next runnable context is the context that is swapped to. 

<h3>Partially Implemented</h3>
thread_yield()/SJF, for some reason it is not calculating the right runtimes for each thread, although the sort function works, each thread has an average time higher than the last, therefore the SJF queue is unchanged. 

<h3>Not Implemented</h3>
Extra Credit is not implemented.

<h3>Tests Performed</h3>
Small tests at each step of building the code were performed. When near completion, the test cases given by hw3texts_shared.txt were used. 
