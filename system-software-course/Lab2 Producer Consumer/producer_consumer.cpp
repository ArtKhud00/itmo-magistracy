#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <sstream>
#include <vector>

pthread_t *thread_ids;

pthread_key_t cur_tid;

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t valueConditional = PTHREAD_COND_INITIALIZER;

typedef struct thread_parametrs {
  thread_parametrs(int sleep_time, int cons_count, bool deb)
      : num(0),
        valueReady(false),
        inputDone(false),
        readyConsumersNumber(0),
        finishedConsumersNumber(0) {
    max_time_sleep = sleep_time;
    consumersCount = cons_count;
    debug = deb;
  }
  int num = 0;
  bool valueReady = false;
  bool inputDone = false;
  bool debug = false;
  int consumersCount;
  int max_time_sleep;
  int readyConsumersNumber = 0;
  int finishedConsumersNumber = 0;
} th_pm;

int get_tid() {  // function creates custom threads ids
  // 1 to 3+N thread ID
  static std::atomic<int> thread_counter = {1};
  int *tid = static_cast<int *>(pthread_getspecific(cur_tid));
  if (tid == NULL) {
    int *new_tid = new int(thread_counter.fetch_add(1));
    pthread_setspecific(cur_tid, new_tid);
    return *new_tid;
  }
  return *(std::atomic<int> *)tid;
}

void tid_destructor(void *ptr) {  // destructor function for custom thread ids
  int *tid = static_cast<int *>(ptr);
  delete tid;
}

int my_rand(int low_val, int high_val) {  // random function
  std::random_device rd;  // Only used once to initialise (seed) engine
  std::mt19937 gen(
      rd());  // Random-number engine used (Mersenne-Twister in this case)
  std::uniform_int_distribution<int> dis(low_val, high_val);
  return dis(gen);
}

void *producer_routine(void *arg) {
  th_pm *par = static_cast<th_pm *>(arg);
  pthread_mutex_lock(&g_mutex);
  while (par->readyConsumersNumber < par->consumersCount) {
    pthread_cond_wait(&consumerCond, &g_mutex);
  }
  pthread_mutex_unlock(&g_mutex);
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  std::string input;
  getline(std::cin, input);
  std::istringstream string_stream(input);
  for (int num; string_stream >> num;) {
    pthread_mutex_lock(&g_mutex);
    par->num = num;
    par->valueReady = true;
    pthread_cond_signal(&valueConditional);
    do {
      pthread_cond_wait(&consumerCond, &g_mutex);
    } while (par->readyConsumersNumber == 0 || par->valueReady);
    pthread_mutex_unlock(&g_mutex);
  }
  pthread_mutex_lock(&g_mutex);
  par->inputDone = true;
  par->valueReady = true;
  do {
    pthread_cond_broadcast(&valueConditional);
    usleep(1);
    pthread_cond_wait(&consumerCond, &g_mutex);
  } while (par->finishedConsumersNumber < par->consumersCount);
  pthread_mutex_unlock(&g_mutex);
  return nullptr;
}

void *consumer_routine(void *arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  th_pm *par = static_cast<th_pm *>(arg);
  int sec = 0;
  int *l_sum = new int(0);
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  while (true) {
    pthread_mutex_lock(&g_mutex);
    par->readyConsumersNumber++;
    pthread_cond_broadcast(&consumerCond);
    do {
      pthread_cond_wait(&valueConditional, &g_mutex);
    } while (!par->valueReady);

    if (!par->inputDone) {
      *l_sum += par->num;
      if (par->debug)
        std::cout << "tid " << get_tid() << ": psum " << *l_sum << std::endl;
      par->readyConsumersNumber--;
      par->valueReady = false;
      pthread_cond_broadcast(&consumerCond);
      pthread_mutex_unlock(&g_mutex);
    } else {
      par->finishedConsumersNumber++;
      pthread_cond_broadcast(&consumerCond);
      pthread_mutex_unlock(&g_mutex);
      return l_sum;
    }
    sec = my_rand(0, par->max_time_sleep);
    usleep(sec * 1000);
  }
}

void *consumer_interruptor_routine(void *arg) {
  th_pm *par = static_cast<th_pm *>(arg);
  // interrupt random consumer while producer is running
  int rand = 0;
  while (!par->inputDone) {
    rand = my_rand(2, par->consumersCount + 1);
    pthread_t cancel_id = thread_ids[rand];
    pthread_testcancel();
    pthread_cancel(cancel_id);
  }
  return nullptr;
}

// the declaration of run threads can be changed as you like
int run_threads(bool debug, int n_threads, int max_time_sleep) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  th_pm par(max_time_sleep, n_threads, debug);
  pthread_key_create(&cur_tid, tid_destructor);
  thread_ids = new pthread_t[2 + n_threads];
  pthread_create(&thread_ids[0], NULL, producer_routine, &par);
  for (auto i = 0; i < n_threads; ++i) {
    pthread_create(&thread_ids[2 + i], NULL, consumer_routine, &par);
  }
  pthread_create(&thread_ids[1], NULL, consumer_interruptor_routine, &par);
  auto final_sum = std::make_unique<int>(0);
  for (int i = 0; i < n_threads; ++i) {
    void *result_ptr;
    pthread_join(thread_ids[2 + i], &result_ptr);
    *final_sum += *(static_cast<int *>(result_ptr));
    delete static_cast<int *>(result_ptr);
  }
  pthread_join(thread_ids[0], nullptr);
  pthread_join(thread_ids[1], nullptr);
  delete[] thread_ids;
  return *final_sum;
}
