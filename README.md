# PtrVec-Vs-UnionVec

Some results:

Vec length: 1000000, iterations: 5

struct sizes: 1, 2, 3 (doubles)
-------- 
- vec of pointers took: 8696.48us 
- vec of unions took: 5803.56us 


struct sizes: 3, 6, 9 (doubles)
-------- 
- vec of pointers took: 11005.4us 
- vec of unions took: 7206.86us


struct sizes: 9, 18, 27 (doubles)
-------- 
- vec of pointers took: 19409.2us 
- vec of unions took: 15706.5us


struct sizes: 27, 54, 81 (doubles)
-------- 
- vec of pointers took: 50261.4us 
- vec of unions took: 49376us 


struct sizes: 81, 162, 243 (doubles)
-------- 
- vec of pointers took: 160235us 
- vec of unions took: 190327us