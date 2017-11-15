// Copyright (c) 2017, Alireza Abyaneh et al. All rights reserved.
// Please see LICENSE file

namespace preprocess {
  
  ////////
  // postorder traversal of the tree
  ////////
  void postorder_traverse(Node* root, int* tr_post, int* node_ids)
  {
    if (root) {
      if (root->children_number > 0) {
        for (int i = 0; i < root->children_number; ++i) {
          postorder_traverse(root->children.at(i), tr_post, node_ids);
        }
      }
      
      root->id = *node_ids;
      tr_post[(*node_ids)] = root->label_id;
      ++(*node_ids);
    }
  }
  
  ////////
  // Left most leaf descendant computation
  ////////
  void lmld(Node* root, int* l)
  {
    for (int i = 1; i <= root->children_number; ++i) {
      lmld(root->children.at(i - 1), l);
    }
    
    if (root->children_number == 0) {
      // is a leaf
      l[root->id] = root->id;
    } else {
      l[root->id] = l[root->children.at(0)->id];
    }
  }
  
  ////////
  // key_root nodes of the tree
  ////////
  void key_roots(int* kr, int* l, int leaf_count, int tree_size)
  {
    int* visit = new int[tree_size + 1]();
    
    int k = leaf_count;
    int i = tree_size;
    while (k >= 1) {
      if (visit[l[i]] == 0) {
        kr[k--] = i;
        visit[l[i]] = 1;
      }
      i -= 1;
    }
    
    delete[] visit;
  }
  
};
