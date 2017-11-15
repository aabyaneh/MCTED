// Copyright (c) 2017, Alireza Abyaneh. All rights reserved.
// Please see LICENSE file

namespace parser {
  int num;
  int labelid;
  
  typedef std::unordered_map<std::string,int> hash_table_type;
  
  Node* tree_create(char* str, hash_table_type& hash_table, int& leaves_cnt)
  {
    int length = std::strlen(str);
    int first = 1;
    int size;
    int isLeave = 0;
    Node* root;
    std::vector<Node*> stack;
    std::string label = "";
    
    int i = 0;
    while (i < length) {
      if (str[i] == '{') {
        isLeave = 1;
        i++;
        continue;
      } else if (str[i] == '}') {
        if (isLeave) {
          leaves_cnt++;
          isLeave = 0;
        }
        
        size = stack.size();
        if(size > 1) {
          stack.at(size - 2)->children.push_back(stack.at(size - 1));
          stack.at(size - 2)->children_number++;
          stack.at(size - 1)->parent = stack.at(size - 2);
          stack.pop_back();
        }
        i++;        
      } else {
        if(str[i] == '\n')
          break;
        
        while (str[i] != '{' && str[i] != '}') {
          label += str[i];
          i++;
        }
        
        num++;
        Node* tmp = new Node();
        tmp->children_number = 0;
        stack.push_back(tmp);
        
        if (!hash_table.count(label)) {
          hash_table.emplace(label, labelid);
          tmp->label_id = labelid++;
        } else {
          tmp->label_id = hash_table[label];
        }
        
        if(first) {
          root = tmp;
          first = 0;
        }
        
        label = "";
      }
    }
    
    return root;
  }
  
};
