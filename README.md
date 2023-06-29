# ump
workqueue in user space

# description
Based on multi-thread in user space.
You can put your function into ump, so the function is running parallelly. 
Thus, you can take advantage of multi-core CPUs.

# performance
1. without ump
> The result of test is "delay is 47921"

2. with ump
> The result of test is "delay is 9799"

3. result
> The performance is improved by 389%

# build
## for so
gcc -fpic -shared -o libump.so ump.c

## for test
gcc main.c -L. -lump -lpthread

# run test
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD

## without ump
./a.out

## with ump
./a.out 1

