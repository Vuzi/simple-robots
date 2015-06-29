# To compile the test
gcc test_worker.c workers.c list.c -o test_worker -Wall -O3 -std=c99 -pthread

# To check for memory leaks
# valgrind --leak-check=full --show-reachable=yes  ./test_worker

