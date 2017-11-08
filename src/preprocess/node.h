
struct Node {
  int id;
  int label_id;
  int children_number;
  int kr_idx;
  bool is_kr = false;
  Node* parent = NULL;
  std::vector<Node*> children;
};
