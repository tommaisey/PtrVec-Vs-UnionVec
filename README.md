# Vec of sum types vs Vec of pointers

I wanted to investigate the performance of two approaches to a vector of polymorphic objects. First, a vector of pointers to a base type. Second, a vector of tagged unions, laid out contiguously in memory. 

It's likely the main differentiator of performance will be the difference between the cost of memory fetches (i.e. following the pointers) or branch mispredictions (when switching on the union's tag).

The first pattern is has a long history of use in C++. The second pattern becomes easier with std::variant in C++17, or Rust's enum types.

Method
---------
Define 3 structs of varying size, which each implement an interface to do a simple calculation.

Create a vector of pointers to the base type, and a vector of tagged unions. The order of the objects is random, to confound branch prediction.

- **Note**: We also allocate some other objects as we create our pointer vec, to fragment heap allocations for more realistic access patterns.
- **Note**: I wrote my own simple union class, as there's no C++17 std::variant in Xcode's stdlib yet. It’s also nice to have all the machinery visible - I’m more interested in the performance of memory access patterns than std::variant specifically.

We iterate each vector 5 times, doing the calculation on each element, and average the time taken. I did this with various struct sizes.

In my tests, the union did very well up to pretty large struct sizes (~150 doubles), so it seems that even missed branches are less expensive than memory fetches.

Results
-----------
On my 2017 iMac, 3.6 GHz Intel Core i7, with a vector length of 1000000, 5 averaged iterations:

##### struct sizes: 1, 2, 3 (doubles)
- vec of pointers took: 8696.48us 
- vec of unions took: 5803.56us 


##### struct sizes: 3, 6, 9 (doubles)
- vec of pointers took: 11005.4us 
- vec of unions took: 7206.86us


##### struct sizes: 9, 18, 27 (doubles)
- vec of pointers took: 19409.2us 
- vec of unions took: 15706.5us


##### struct sizes: 27, 54, 81 (doubles)
- vec of pointers took: 50261.4us 
- vec of unions took: 49376us 


##### struct sizes: 81, 162, 243 (doubles)
- vec of pointers took: 160235us 
- vec of unions took: 190327us