// Copyright (c) 2017, Alireza Abyaneh et al. All rights reserved.
// Please see LICENSE file

#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <atomic>
#include <pthread.h>

#include "src/preprocess/node.h"
#include "src/preprocess/parser.h"
#include "src/zhsh/zh_sh.h"
#include "src/preprocess/preprocess.h"
#include "src/zhsh/zh_sh_serial.h"
#include "src/zhsh/zh_sh_parallel.h"

Node* tree1;
Node* tree2;
struct timeval tv_begin;
struct timeval tv_end;
double time_elapsed_in_mcseconds;

void preprocessor(char* tree1_str, char* tree2_str)
{
  tree1_size = 0;
  tree2_size = 0;
  leaves_cnt1 = 0;
  leaves_cnt2 = 0;
  parser::labelid = 1;
  parser::num = 0;
  parser::hash_table_type hash_table;

  tree1 = parser::tree_create(tree1_str, hash_table, leaves_cnt1);
  tree1_size = parser::num;  parser::num = 0;
  tree2 = parser::tree_create(tree2_str, hash_table, leaves_cnt2);
  tree2_size = parser::num;

  int node_ids = 1;
  tree1_postorder = new int[tree1_size + 1];
  preprocess::postorder_traverse(tree1, tree1_postorder, &node_ids);
  node_ids = 1;
  tree2_postorder = new int[tree2_size + 1];
  preprocess::postorder_traverse(tree2, tree2_postorder, &node_ids);

  l1 = new int[tree1_size + 1];
  l2 = new int[tree2_size + 1];
  preprocess::lmld(tree1, l1);
  preprocess::lmld(tree2, l2);

  kr1 = new int[leaves_cnt1+1];
  kr2 = new int[leaves_cnt2+1];
  preprocess::key_roots(kr1, l1, leaves_cnt1, tree1_size);
  preprocess::key_roots(kr2, l2, leaves_cnt2, tree2_size);
}

void zs_serial()
{
  float result;

  gettimeofday(&tv_begin,NULL);
  result = zss::tree_dist(tree1_size + 1, tree2_size + 1);
  gettimeofday(&tv_end,NULL);

  time_elapsed_in_mcseconds = (tv_end.tv_sec - tv_begin.tv_sec) * 1000000 + (tv_end.tv_usec - tv_begin.tv_usec);

  std::cout << "######################################"<< std::endl;
  std::cout << "***** Serial *****" << std::endl;
  std::cout << "######################################"<< std::endl;
  std::cout << "**** tree_1 size: " << tree1_size << std::endl;
  std::cout << "**** tree_2 size: " << tree2_size << std::endl;
  std::cout << "tree edit distance: " << result << std::endl;
  std::cout << "time (second): " << time_elapsed_in_mcseconds / 1000000 << std::endl;
  std::cout << std::endl;
}

void zs_parallel() {
  float result;

  zsp::y_td = tree2_size + 1;

  gettimeofday(&tv_begin,NULL);
  result = zsp::workload(tree1, tree2);
  gettimeofday(&tv_end,NULL);

  time_elapsed_in_mcseconds = (tv_end.tv_sec - tv_begin.tv_sec) * 1000000 + (tv_end.tv_usec - tv_begin.tv_usec);

  std::cout << "######################################"<< std::endl;
  std::cout << "***** Parallel *****" << std::endl;
  std::cout << "######################################"<< std::endl;
  std::cout << "**** tree_1 size: " << tree1_size << std::endl;
  std::cout << "**** tree_2 size: " << tree2_size << std::endl;
  std::cout << "**** num of threads in pool: " << zsp::num_of_threads_in_pool << std::endl;
  std::cout << "tree edit distance: " << result << std::endl;
  std::cout << "time (second): " << time_elapsed_in_mcseconds / 1000000 << std::endl;
  std::cout << std::endl;
}

int main (int argc, char* argv[])
{
  /*  key_root pairs with fd matrices of less than 'subtrees_nodes_num_threshold' elements
   will compute serially.  */
  zsp::subtrees_nodes_num_threshold = 50;


  std::string input;
  int error = 0;
  int serial = 0;
  int parallel = 0;
  int i = 1;
  while(i < argc) {
    if (!strcmp(argv[i], "-i")) {
      if (i + 1 < argc) {
        input = argv[i + 1];
        i++;
      } else
      error = 1;
    } else if (!strcmp(argv[i], "-s")) {
      serial = 1;
    } else if (!strcmp(argv[i], "-p")) {
      if (i + 1 < argc) {
        zsp::num_of_threads_in_pool = atoi(argv[i + 1]);
        parallel = 1;
        i++;
      } else
      error = 1;
    } else if (!strcmp(argv[i], "-h")) {
      error = 1;
      break;
    }
    i++;
  }

  std::ifstream input_trees;
  if (error == 0 && input.length() != 0) {
    input_trees.open(input);
    std::string line1;
    std::string line2;
    std::getline(input_trees, line1);
    std::getline(input_trees, line2);

    preprocessor(&line1[0], &line2[0]);

    if (serial == 1)
    zs_serial();
    if (parallel == 1)
    zs_parallel();

    delete[] tree1_postorder;
    delete[] tree2_postorder;
    delete[] l1;
    delete[] l2;
    delete[] kr1;
    delete[] kr2;

    input_trees.close();

  } else {
    std::cout << "HELP:"<< std::endl;
    std::cout << "  " << "-i input" << "            " << "Input trees in a file called input"<< std::endl;
    std::cout << "  " << "-s" << "                  " << "Serial execution"<< std::endl;
    std::cout << "  " << "-p num_cores" << "        " << "Parallel execution"<< std::endl;
    std::cout << "  " << "-s -p num_cores" << "     " << "Serial and Parallel execution"<< std::endl;
    std::cout << "  " << "example:" << "            " << "./mcted -i trees.txt -s -p 4"<< std::endl;
  }

  return 0;
}
