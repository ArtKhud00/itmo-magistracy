#include <cstring>
#include <iostream>
#include "producer_consumer.h"

bool debug = false;  // debug flag
int n_threads;       // number of threads
int max_time_sleep;  // max time in seconds consumers threads sleeping

void parse_func(char **args) {
  n_threads = atoi(args[0]);
  max_time_sleep = atoi(args[1]);
}

int main(int argc, char **argv) {
  // main func parse arguments
  switch (argc) {
    case 3:
      parse_func(argv + 1);
      break;
    case 4:
      if (strcmp(argv[1], "--debug") == 0) debug = true;
      parse_func(argv + 2);
      break;
    default:
      std::cout << "usage: <./posix [--debug] N_consumer_threads "
                   "consumer_max_s_sleep"
                << std::endl;
      return 1;
  }

  std::cout << run_threads(debug, n_threads, max_time_sleep) << std::endl;
  return 0;
}
