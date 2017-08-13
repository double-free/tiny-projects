问题
---
实现一个 1000x1000 的矩阵乘法。

定义与初始化
```c++
#include <chrono>
#include <iostream>

const int MATRIX_SIZE = 1000;
double matrix1[MATRIX_SIZE][MATRIX_SIZE];
double matrix2[MATRIX_SIZE][MATRIX_SIZE];
double result[MATRIX_SIZE][MATRIX_SIZE];

void initMat() {
  std::random_device rd;
  std::default_random_engine dre(rd());
  std::uniform_real_distribution<double> urd(0.0, 1.0);
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      matrix1[i][j] = urd(dre);
      matrix2[i][j] = urd(dre);
    }
  }
}
```
无优化实现
---
```c++
void multiple_no_opt() {
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      for (int k = 0; k < MATRIX_SIZE; ++k) {
        result[i][j] += matrix1[i][k] * matrix2[k][j];
      }
    }
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "No opt: " << diff.count() << " seconds"
            << std::endl; // 5.14706s
}
```
第一层境界
---
利用缓存优化
```c++
void multiple_transpose_opt() {
  // 栈上分配的矩阵是一行一行连续存储的
  // 未优化版本中，matrix2[k][j] 显然没有访问连续的内存
  // 将其转置后就是连续的内存了，可以充分利用 cache
  auto transpose_start = std::chrono::steady_clock::now();
  double transposed_matrix2[MATRIX_SIZE][MATRIX_SIZE];
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      transposed_matrix2[i][j] = matrix2[j][i];
    }
  }
  auto transpose_end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = transpose_end - transpose_start;
  std::cout << "transpose cost: " << diff.count() << " seconds" << std::endl; // 0.0048

  auto multiple_start = std::chrono::steady_clock::now();
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      for (int k = 0; k < MATRIX_SIZE; ++k) {
        result[i][j] += matrix1[i][k] * transposed_matrix2[j][k];
      }
    }
  }
  auto multiple_end = std::chrono::steady_clock::now();
  diff = multiple_end - multiple_start;
  std::cout << "transposed multiple cost: " << diff.count() << " seconds"
            << std::endl; // 3.4747
}
```
优化了 30% 左右的时间，代价是拷贝了一个矩阵的额外空间。

第二层境界
---
细致到 L1 Data 缓存进行优化。可以避免使用额外的空间。
在 Linux 下可以利用以下命令查询得到 L1 Data 缓存的 linesize。linesize 可以理解为存储的相邻内存区域的大小。
```
getconf LEVEL1_DCACHE_LINESIZE
```
在我的机器上是 64 Bytes。那么相当于可以存 8 个 double。也就是说对于 matrix2，在使用 (0,0) 位置的元素时，(0,1), (0,2) ...(0,7) 都被加载到了缓存。于是我们就可以考虑将大矩阵分割为小矩阵进行，充分利用缓存，并且不需要额外作转置。
```c++
void multiple_L1d_opt() {
  auto multiple_start = std::chrono::steady_clock::now();
  for (int i = 0; i < MATRIX_SIZE; i += SM) {
    for (int j = 0; j < MATRIX_SIZE; j += SM) {
      for (int k = 0; k < MATRIX_SIZE; k += SM) {
        // 分成了小矩阵，每个 slice 是长度为 SM 的一行
        double *slice_res = &result[i][j];
        double *slice_mat1 = &matrix1[i][k];
        double *slice_mat2 = &matrix2[k][j];

        for (int ii = 0; ii < SM;
             ++ii, slice_res += MATRIX_SIZE, slice_mat1 += MATRIX_SIZE) {
          for (int kk = 0; kk < SM; ++kk, slice_mat2 += MATRIX_SIZE) {
            for (int jj = 0; jj < SM; ++jj) {
              slice_res[jj] += slice_mat1[kk] * slice_mat2[jj];
            }
          }
          // 重置 slice_mat2
          slice_mat2 = &matrix2[k][j];
        }
      }
    }
  }
  auto multiple_end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = multiple_end - multiple_start;
  std::cout << "L1 data opt multiple cost: " << diff.count() << " seconds"
            << std::endl; // 3.4747
}
```
