// Copyright (c) 2017, Alireza Abyaneh. All rights reserved.
// Please see LICENSE file

struct Node {
  int id;
  int label_id;
  int children_number;
  int kr_idx;
  bool is_kr = false;
  Node* parent = NULL;
  std::vector<Node*> children;
};
