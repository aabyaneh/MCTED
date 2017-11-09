namespace zss {
  
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
  
  float tree_dist(int size1, int size2)
  {
    // tree distance matrix
    float* td = new float[size1 * size2];
    // forest distance matrix
    float* fd = new float[size1 * size2];
    
    int di;
    int dj;
    int H_base;
    int W_base;
    int ren;
    int i;
    int j;
    
    fd[0] = 0;
    for (di = 1; di < size1; di++) {
      fd[di * size2] = fd[(di - 1) * size2] + del_cost;
    }
    
    for (dj = 1; dj < size2; dj++) {
      fd[dj] = fd[dj - 1] + ins_cost;
    }
    
    for (int i1 = 1; i1 < leaves_cnt1 + 1; i1++) {
      i = kr1[i1];
      H_base = l1[i] - 1;
      
      for (int j1 = 1; j1 < leaves_cnt2 + 1; j1++) {
        j = kr2[j1];
        W_base = l2[j] - 1;
        
        for (di = l1[i]; di <= i; di++) {
          for (dj = l2[j]; dj <= j; dj++) {
            if (l1[di] == l1[i] && l2[dj] == l2[j]) {
              ren = 0;
              if (tree1_postorder[di] != tree2_postorder[dj]) {
                ren = up_cost;
              }
              
              min(fd[(di - 1 - H_base) * size2 + (dj - W_base)] + del_cost, fd[(di - H_base) * size2 + (dj - 1 - W_base)] + ins_cost,
                  fd[(di - 1 - H_base) * size2 + (dj - 1 - W_base)] + ren,
                  fd[(di - H_base) * size2 + (dj - W_base)]);
              
              td[di * size2 + dj] = fd[(di - H_base) * size2 + (dj - W_base)];
              
            }
            else
            {
              min(fd[(di - 1 - H_base) * size2 + (dj - W_base)] + del_cost, fd[(di - H_base) * size2 + (dj - 1 - W_base)] + ins_cost,
                  fd[(l1[di] - 1 - H_base) * size2 + (l2[dj] - 1 - W_base)] + td[di * size2 + dj],
                  fd[(di - H_base) * size2 + (dj - W_base)]);
            }
          }
        }
        
      }
    }
    
    float ted = td[tree1_size * size2 + tree2_size];
    delete[] td;
    delete[] fd;
    
    return ted;
  }
  
}
