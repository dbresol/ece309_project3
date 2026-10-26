# Project 3 Design Log
**Author:** Daniel Bresolin · **P2 code used:** my own
**Tools used:** Gemini was used to fix bugs in the calculator.

## 1. Hash table
I used chaining for the hash table, and I used the FNV-1a hashing algorithm as provided in the project spec. I used chaining because I expected that the implementation would be easier than open addressing. Additionally, the bucket chosen by the mapping is selected first and then the linked list (or vector, in this case) is accessed, which allows the selection of the bucket and the traversal through the linked list to be done with nested for loops. I used a load factor threshold of 0.9 because the test cases only every add a single tool at a time, making a high load factor hash table which resizes itself relatively infrequently a good choice. My resize policy doubled the number of buckets when the load factor threshold was reached because this would indicate a high number of tools on average inside each bucket. The output for the 10000-tool test was the following: 

	bucket_count = 11330; load_factor = 0.882613; max_probe_length = 6

The output for the alternate hash was the following: 
	
	Longest alternate hash chain length: 9000

add() is amortized O(1) because *the average time it takes to resize is constant. The time it takes to perform the resizing operation is inversely proportional to how often they happen, making the overall time constant.*

## 2. Pointer stability

During the resizing process, an empty table with the new size is created. Each unique_ptr in the original table is moved to the new one using std::move() so that the pointer object is never deleted, and as such the Tool* pointer stored in each unique_ptr is never moved. Because find() returns the Tool* pointer and not the unique_ptr, the Tool* pointer survives the resize.

## 3. Calculator trace

First, the value 2 is pushed to the values stack. Then, the operand * is pushed to the ops stack. Because the ops stack is empty at that point, no operation is performed. Then, the left parentheses is pushed to the ops stack. Then, the value 3 is pushed to the values stack. Then, the operand + is pushed to the ops stack, and no operation is performed because the operand pushed before it was a left parentheses, indicating that another value is needed. Then, the value 4 is pushed to the values stack. Then, the right parentheses causes the operation function to be called repeatedly until the left parentheses is encountered. In this case, the values 4 and 3 and the operand + are popped, and the value 3 + 4 is calculated, with the result 7 being pushed to the values stack. Then, the operand – is encountered. Because the top of the ops stack is *, and because multiplication has a higher precedence than subtraction, multiplication is performed first. Values 7 and 2 and the operand * are popped, and the value 2 * 7 is calculated, with the result 14 being pushed to the values stack. Then, the operand – is pushed onto the ops stack. Then, the value 5 is pushed onto the values stack. The last token has been pushed, so all remaining operands are operated upon. The values 5 and 14 and the operand – are popped, and the value 14 – 5 is calculated, with the result 9 being pushed to the values stack. This is the only value left on the values stack and there are no operands on the ops stack, so the result 9 is returned successfully.

## 4. Errors as values

Tools, resolve() and calc_evaluate() return errors because these are errors of usage instead of programming errors. If an equation entered as the tokens parameter for calc_evaluate() is formatted incorrectly, this is user error and they should be informed. resolve() returns an error when the given path is not allowed because actually trying to work with the given path and encountering an exception could be catastrophic, and as such should only be allowed when specific requirements are met. Errors caused by a tool are expected in the normal functioning of the agent, and as such should also be handled by the agent with an error instead of an exception. An exception would be permitted if something happens which can not or should not be handled, such as if the running device runs out of memory.

## 5. What I would design differently

