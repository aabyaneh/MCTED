// Copyright (c) 2017, Alireza Abyaneh. All rights reserved.
// Please see LICENSE file

#include "../thread_pool/thpool.h"

namespace zsp {
  
  int* subtree_nodes1;
  int* subtree_nodes2;
  
  int* kr1_order;
  int* kr2_order;
  
  struct parents {
    int parent;
    int parent_idx;
  };
  struct parents* parents_1;
  struct parents* parents_2;
  
  int subtrees_nodes_num_threshold;
  
  // data structure for TD threads argumants
  struct arg_td_struct {
    int arg1;
    int arg2;
    int arg3;
    int arg4;
  };
  
  // for controlling jobs
  bool fin;
  pthread_mutex_t mutex_c;
  pthread_cond_t cv_c;
  
  // tree_distance matrix
  float* td;
  // for controlling independent key_root pairs
  std::atomic<int>* ctd;
  // width of tree_distance matrix
  int y_td;
  
  // thread pool
  pool* thpool;
  int num_of_threads_in_pool;
  
  ////
  // Minimum of 3 doubles
  ////
  void min(float in1, float in2, float in3, float& res)
  {
    res = in1;
    if (in2 < res) {
      res = in2;
    }
    if (in3 < res) {
      res = in3;
    }
  }
  
  ////
  // for each key_root node, returns the number of node in postorder traverse
  // which is the first key_root ancestor of it
  ////
  void kr_parent(Node* root, int* kr, struct parents* parent, int leaves_cnt)
  {
    int id = root->id;
    for (int cnt = 1; cnt < leaves_cnt + 1; cnt++) {
      if (kr[cnt] == id) {
        root->is_kr = true;
        root->kr_idx = cnt;
        
        if(root->parent) {
          Node* anc = root->parent;
          while(!anc->is_kr) {
            anc = anc->parent;
          }
          parent[cnt].parent = anc->id;
          parent[cnt].parent_idx = anc->kr_idx;
        }
        else {
          parent[cnt].parent = 0;
          parent[cnt].parent_idx = 0;
        }
        break;
      }
    }
    
    if (root->children_number == 0) {
      return;
    }
    
    for (int i = 1; i <= root->children_number; ++i) {
      kr_parent(root->children.at(i - 1), kr, parent, leaves_cnt);
    }
  }
  
  /////
  /// The thread for execution of one pair of key_root_nodes
  /////
  void thread_td(void* argument, int& id)
  {
    struct arg_td_struct *args = (struct arg_td_struct *)argument;
    int i = args->arg1; // key_root node of tree1
    int j = args->arg2; // key_root node of tree2
    int i1 = args->arg3; // key_root node index of tree1
    int j1 = args->arg4; // key_root node index of tree2
    
    bool is_par;
    bool is_par2;
    int H;
    int W;
    int H_base;
    int W_base;
    int di;
    int dj;
    int ren;
    
    while(true)
    {
      H = i - l1[i] + 2; // Height of fd matrix
      W = j - l2[j] + 2; // Width of fd matrix
      H_base = l1[i] - 1; // Base index of matrix (Height)
      W_base = l2[j] - 1; // Base index of matrix (Width)
      
      // forest distance matrix
      float* fd = new float[H * W];
      
      di = 0;
      dj = 0;
      fd[0] = 0;
      for (dj = l2[j]; dj <= j; ++dj) {
        fd[dj - W_base] = fd[dj - 1 - W_base] + ins_cost;
      }
      
      for (di = l1[i]; di <= i; ++di) {
        fd[(di - H_base) * W] = fd[(di - H_base - 1) * W] + del_cost;
        for (dj = l2[j]; dj <= j; ++dj) {
          if (l1[di] == l1[i] && l2[dj] == l2[j]) {
            ren = 0;
            if (tree1_postorder[di] != tree2_postorder[dj]) {
              ren = up_cost;
            }
            
            min(fd[(di - 1 - H_base) * W + (dj - W_base)] + del_cost, fd[(di - H_base) * W + (dj - 1 - W_base)] + ins_cost,
                fd[(di - 1 - H_base) * W + (dj - 1 - W_base)] + ren,
                fd[(di - H_base) * W + (dj - W_base)]);
            
            td[di * y_td + dj] = fd[(di - H_base) * W + (dj - W_base)];
            
          }
          else {
            min(fd[(di - 1 - H_base) * W + (dj - W_base)] + del_cost, fd[(di - H_base) * W + (dj - 1 - W_base)] + ins_cost,
                fd[(l1[di] - 1 - H_base) * W + (l2[dj] - 1 - W_base)] + td[di * y_td + dj],
                fd[(di - H_base) * W + (dj - W_base)]);
          }
        }
      }
      
      delete[] fd;
      
      ////
      // Updating ctd matrix for independent key_roots
      ////
      is_par = false;
      is_par2 = false;
      if (j1 < leaves_cnt2)
      {
        ctd[i * y_td + parents_2[j1].parent].fetch_add(-1);
        int val = ctd[i*y_td + parents_2[j1].parent].load();
        while(val == 0)
        {
          if(ctd[i * y_td + parents_2[j1].parent].compare_exchange_weak(val, -2))
          {
            is_par = true;
          }
        }
      }
      
      if(i1 < leaves_cnt1)
      {
        ctd[parents_1[i1].parent * y_td + j].fetch_add(-1);
        int val = ctd[parents_1[i1].parent * y_td + j].load();
        while(val == 0)
        {
          if(ctd[parents_1[i1].parent * y_td + j].compare_exchange_weak(val, -2))
          {
            is_par2 = true;
          }
        }
      }
      
      if(i1 == leaves_cnt1 && j1 == leaves_cnt2)
      {
        pthread_mutex_lock(&mutex_c);
        fin = true;
        pthread_cond_signal(&cv_c);
        pthread_mutex_unlock(&mutex_c);
      }
      
      if(is_par)
      {
        if(is_par2)
        {
          arg_td_struct* thread_args = new arg_td_struct[1];
          thread_args[0].arg1 = parents_1[i1].parent;
          thread_args[0].arg2 = j;
          thread_args[0].arg3 = parents_1[i1].parent_idx;
          thread_args[0].arg4 = j1;
          
          enqueue(thpool,thpool->jobQ[id], &thread_args[0]);
          
          is_par2 = false;
        }
        
        j = parents_2[j1].parent;
        j1 = parents_2[j1].parent_idx;
      }
      else if (is_par2)
      {
        i = parents_1[i1].parent;
        i1 = parents_1[i1].parent_idx;
      }
      else
      {
        break;
      }
      
    }
    
    delete[] args;
  }
  
  
  ////////
  //  Different key-root pairs
  ////////
  float tree_dist(int size1, int size2)
  {
    fin = false;
    
    ////
    // Serial computation for subtrees of small size
    ////
    float* fd = new float[size1 * size2];
    
    int di = 0;
    int dj = 0;
    int ren;
    int H_base;
    int W_base;
    int cont = 0;
    bool end = true;
    int i1;
    int j1;
    int i;
    int j;
    
    fd[0] = 0;
    for (di = 1; di < size1; ++di) {
      fd[di * y_td] = fd[(di - 1) * y_td] + del_cost;
    }
    
    for (dj = 1; dj < size2; ++dj) {
      fd[dj] = fd[dj - 1] + ins_cost;
    }
    
    while(end) {
      end = false;
      for (i1 = 1; i1 < leaves_cnt1 + 1; i1++) {
        for (j1 = 1; j1 < leaves_cnt2 + 1; j1++) {
          i = kr1[i1];
          j = kr2[j1];
          
          if(ctd[i * y_td + j] != 0) {
            continue;
          } else if (subtree_nodes1[i1]*subtree_nodes2[j1] > subtrees_nodes_num_threshold) {
            continue;
          }
          
          end = true;
          cont++;
          
          ctd[i * y_td + j].store(-2);
          ctd[i * y_td + parents_2[j1].parent].fetch_add(-1);
          ctd[parents_1[i1].parent * y_td + j].fetch_add(-1);
          
          H_base = l1[i] - 1;
          W_base = l2[j] - 1;
          for (di = l1[i]; di <= i; ++di) {
            for (dj = l2[j]; dj <= j; ++dj) {
              if (l1[di] == l1[i] && l2[dj] == l2[j]) {
                ren = 0;
                if (tree1_postorder[di] != tree2_postorder[dj]) {
                  ren = up_cost;
                }
                
                min(fd[(di - 1 - H_base) * y_td + (dj - W_base)] + del_cost, fd[(di - H_base) * y_td + (dj - 1 - W_base)] + ins_cost,
                    fd[(di - 1 - H_base) * y_td + (dj - 1 - W_base)] + ren,
                    fd[(di - H_base) * y_td + (dj - W_base)]);
                
                td[di * y_td + dj] = fd[(di - H_base) * y_td + (dj - W_base)];
                
              }
              else {
                min(fd[(di - 1 - H_base) * y_td + (dj - W_base)] + del_cost, fd[(di - H_base) * y_td + (dj - 1 - W_base)] + ins_cost,
                    fd[(l1[di] - 1 - H_base) * y_td + (l2[dj] - 1 - W_base)] + td[di * y_td + dj],
                    fd[(di - H_base) * y_td + (dj - W_base)]);
              }
            }
          }
        }
      }
    }
    delete[] fd;
    
    
    ////
    // Parallel execution
    ////
    thpool = thpool_init(num_of_threads_in_pool, &thread_td);
    
    // thread's queues init
    bool td_cnt = false;
    int cnt = 0;
    for (int i1 = 1; i1 < leaves_cnt1 + 1; i1++) {
      for (int j1 = 1; j1 < leaves_cnt2 + 1; j1++) {
        i = kr1[i1];
        j = kr2[j1];
        
        if (ctd[i * y_td + j] != 0) {
          continue;
        }
        td_cnt = true;
        ctd[i * y_td + j].store(-2);
        
        arg_td_struct* thread_args = new arg_td_struct[1];
        thread_args[0].arg1 = i;
        thread_args[0].arg2 = j;
        thread_args[0].arg3 = i1;
        thread_args[0].arg4 = j1;
        
        enqueue(thpool,thpool->jobQ[cnt], &thread_args[0]);
        cnt = (cnt + 1) % num_of_threads_in_pool;
      }
    }
    
    // wait until the end
    if(td_cnt) {
      pthread_mutex_init(&mutex_c, NULL);
      pthread_cond_init(&cv_c, NULL);
      thpool_start(thpool);
      
      pthread_mutex_lock(&mutex_c);
      while(!fin) {
        pthread_cond_wait(&cv_c, &mutex_c);
      }
      pthread_mutex_unlock(&mutex_c);
      
      thpool_destroy(thpool);
      pthread_mutex_destroy(&mutex_c);
      pthread_cond_destroy(&cv_c);
    }
    
    float ted = td[tree1_size * y_td + tree2_size];
    
    return ted;
  }
  
  
  ////
  // dependency detection
  ////
  float workload(Node* tree1, Node* tree2)
  {
    ////////
    // Preparing data for detection of independent key_root pairs
    ////////
    // temp matrices
    int* tmp1 = new int[tree1_size + 1]();
    int* tmp2 = new int[tree2_size + 1]();
    
    // parents of key_roots
    parents_1 = new parents[leaves_cnt1 + 1]();
    parents_2 = new parents[leaves_cnt2 + 1]();
    kr_parent(tree1, kr1, parents_1, leaves_cnt1);
    kr_parent(tree2, kr2, parents_2, leaves_cnt2);
    
    for (int i = 1; i < leaves_cnt1; i++) {
      tmp1[parents_1[i].parent]++;
    }
    for (int i = 1; i < leaves_cnt2; i++) {
      tmp2[parents_2[i].parent]++;
    }
    
    kr1_order = new int[leaves_cnt1+1]();
    for (int i = 1; i < leaves_cnt1+1; i++) {
      kr1_order[i] = tmp1[kr1[i]];
    }
    
    kr2_order = new int[leaves_cnt2+1]();
    for (int i = 1; i < leaves_cnt2+1; i++) {
      kr2_order[i] = tmp2[kr2[i]];
    }
    delete[] tmp1;
    delete[] tmp2;
    
    td = new float[(tree1_size + 1) * (tree2_size + 1)]();
    ctd = new std::atomic<int>[(tree1_size + 1) * (tree2_size + 1)](); // the matrix for controlling indpendent key_root pairs
    
    for (int i = 1; i < leaves_cnt1 + 1; i++) {
      for (int j = 1; j < leaves_cnt2 + 1; j++) {
        ctd[kr1[i] * y_td + kr2[j]].store(kr1_order[i] + kr2_order[j]);
      }
    }
    
    ////
    // subtrees size
    ////
    int ii;
    int jj;
    subtree_nodes1 = new int[leaves_cnt1 + 1]();
    subtree_nodes2 = new int[leaves_cnt2 + 1]();
    for (int i = 1; i < leaves_cnt1 + 1; i++)
    {
      ii = kr1[i];
      subtree_nodes1[i] = ii - l1[ii] + 2;
    }
    for (int j = 1; j < leaves_cnt2 + 1; j++)
    {
      jj = kr2[j];
      subtree_nodes2[j] = jj - l2[jj] + 2;
    }
    
    float ted = tree_dist(tree1_size + 1, tree2_size + 1);
    
    delete[] parents_1;
    delete[] parents_2;
    delete[] kr1_order;
    delete[] kr2_order;
    delete[] td;
    delete[] subtree_nodes1;
    delete[] subtree_nodes2;
    delete[] ctd;
    
    return ted;
  }
  
}
