
#include <chrono>
#include <iostream>

using namespace std;

/* 递归求解
*  @para1: 目前棋盘上 queen 数量，同时也是目前所在行
*  @para2: 以 bit 标记的有 queen 的列
*  @para3: 以 bit 标记的有 queen 的对角线
            (0,0) 所在格是第 0 条， (SIZE-1, SIZE-1) 所在格是第 2*(SIZE-1) 条
*  @para4: 以 bit 标记的有 queen 的反对角线
            (SIZE-1, 0) 所在格是第 0 条，(0, SIZE-1) 所在格是第 2*(SIZE-1) 条
*/
long queens(int SIZE, int nQueen=0, long cols=0, long diags=0, long r_diags=0) {
  if (nQueen == SIZE) {
    return 1;
  }
  // 目前所在的是第 nQueen 行，第 i 列
  int solution_num = 0;
  for (int i=0; i<SIZE; i++) {
    long col = 1 << i;
    long diag = 1 << (nQueen + i);
    long r_diag = 1 << (SIZE - 1 - nQueen + i);
    if (!(col&cols) && !(diag&diags) && !(r_diag&r_diags)) {
      // 都不冲突
      solution_num += queens(SIZE, nQueen+1, col|cols, diag|diags, r_diag|r_diags);
    }
  }
  return solution_num;
}


int main() {
  cout << "Please input size(positive integer): "<<endl;
  int SIZE;
  cin >> SIZE;
  auto start_time = chrono::steady_clock::now();
  cout << "Solution num: " << queens(SIZE) << endl;
  auto end_time = chrono::steady_clock::now();
  chrono::duration<double> elapsed = end_time - start_time;
  cout << "elapsed time = " << elapsed.count() << " seconds" << endl;
}
