# Questions
- allocating memory in **heap**
Dynamic allocation of address is done in the heap segment of a program during runtime. Heap has nothing to do with the heap data structure, they are two distinct words used in specific contexts. The opposite of heap is the call stack segment which is used for evaluating and storing results of the function calls which uses static memory addresses. A stack frame - which contains the set of values used or returned during a function call - is created each time a new function call is made. Stack and heap grows in the opposite direction, where the stack typically starts from max size to min size and the heap from min to max. If by any chance both the stack and the heap pointer that points to the top level addresses of the two regions meet each other, that implies there's no free memory available to use for the program.

- stream buffer
