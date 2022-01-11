prodcon.cpp is a solution to the Producer-Consumer problem/Bounded-Buffer problem in C++. This project is based on multi-process synchronization where two processes - 
producer and consumer share a common, fixed-size buffer used as a queue. The producer acts as a server that accepts the incoming transactions and adds them to the 
queue of work to be done. The consumers are each a thread looking for work from the server to execute. The solution uses semaphores to ensure synchronization.

tands.cpp consists of two functions Trans() and Sleep(). Trans() simulates processing a transaction. It does not simulate this by using sleep -- that would free up the CPU. 
Thus, Trans() does a silly computation to use up CPU cycles. Note that the computation has to produce some result. If it doesn't, then a smart compiler will notice this 
and delete (optimize) the code! The loop's computation (to waste time) is used to modify TransSave.TransSave is added to the wait time in Sleep() -- a few billionths of a 
second. By doing this, something "real" comes from the computation, and the compiler is fooled.
                                                    

