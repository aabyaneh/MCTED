// Copyright (c) 2017, Alireza Abyaneh. All rights reserved.
// Please see LICENSE file

struct node_t {
  void*  arg;
  node_t* next;
};

struct queue_t {
  int q_id;
  node_t* head;
  node_t* tail;
  pthread_mutex_t h_lock;
  pthread_mutex_t t_lock;
};

struct pool {
  int num_threads;
  void (*function)(void*, int&);
  struct thread* threads;
  struct queue_t** jobQ;
  bool end;
};

struct thread {
  int id;
  pthread_t pthread;
  pool* thpool_p;
};

void init_q(queue_t* Q)
{
  node_t* node = new node_t();
  node->next = NULL;
  Q->head = node;
  Q->tail = node;
  pthread_mutex_init(&Q->h_lock, NULL);
  pthread_mutex_init(&Q->t_lock, NULL);
}

void enqueue(pool* thpool_p, queue_t* Q, void* arg_p)
{
  node_t* node = new node_t();
  node->arg = arg_p;
  node->next = NULL;
  //pthread_mutex_lock(&Q->t_lock);
  Q->tail->next = node;
  Q->tail = node;
  //pthread_mutex_unlock(&Q->t_lock);
}

bool dequeue(queue_t* Q, node_t* item)
{
  node_t* node;
  node_t* new_head;
  pthread_mutex_lock(&Q->h_lock);
  node = Q->head;
  new_head = node->next;
  if(new_head == NULL) {
    pthread_mutex_unlock(&Q->h_lock);
    return false;
  }
  
  item->arg = new_head->arg;
  Q->head = new_head;
  pthread_mutex_unlock(&Q->h_lock);
  
  delete node;
  return true;
}

void* thread_do(void* th)
{
  struct thread* thread_p = (struct thread*) th;
  
  pool* thpool_p = thread_p->thpool_p;
  
  int num_thr = thpool_p->num_threads;
  int id = thread_p->id;
  void (*func_buff)(void* arg, int& id);
  void* arg_buff;
  
  node_t* item = new node_t();
  while(true) {
    if(thpool_p->end) {
      break;
    }
    
    if(!dequeue(thpool_p->jobQ[id], item)) {
      id = (++id) % (num_thr);
      
      if(!thpool_p->end) {
        continue;
      }
    }
    
    if(item->arg) {
      thpool_p->function(item->arg, thread_p->id);
    }
    item->arg = NULL;
    id = thread_p->id;
  }
  
  delete item;
  return NULL;
}


struct pool* thpool_init(int num_threads, void (*function)(void* arg, int& id))
{
  // create the thread pool
  pool* thpool_p = new pool();
  thpool_p->num_threads = num_threads;
  thpool_p->end = false;
  thpool_p->function = function;
  
  // initialise the job queues of the pool
  thpool_p->jobQ = new queue_t*[num_threads + 1];
  for (int i = 0; i <= num_threads; i++) {
    thpool_p->jobQ[i] = new queue_t();
    init_q(thpool_p->jobQ[i]);
    thpool_p->jobQ[i]->q_id = i;
  }
  
  // initialize the threads of the pool
  thpool_p->threads = new thread[num_threads];
  
  return thpool_p;
}

void thpool_start(struct pool* thpool_p)
{
  // create the threads in the pool
  for (int i = 0; i < thpool_p->num_threads; i++) {
    thpool_p->threads[i].thpool_p = thpool_p;
    thpool_p->threads[i].id = i;
    
    pthread_create(&thpool_p->threads[i].pthread, NULL, &thread_do, &thpool_p->threads[i]);
  }
}

void thpool_destroy(pool* thpool_p)
{
  thpool_p->end = true;
  
  bool result_code;
  for(int i = 0; i < thpool_p->num_threads; i++) {
    result_code = pthread_join(thpool_p->threads[i].pthread, NULL);
    assert(0 == result_code);
  }
  
  for(int i = 0; i < thpool_p->num_threads; i++) {
    delete thpool_p->jobQ[i];
  }
  
  delete[] thpool_p->jobQ;
  delete[] thpool_p->threads;
  delete thpool_p;
}
