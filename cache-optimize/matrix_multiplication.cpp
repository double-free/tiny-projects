// g++ -std=c++0x -DCLS=$(getconf LEVEL1_DCACHE_LINESIZE) -o matrix
// matrix_multiplication.cpp
#include <chrono>
#include <iostream>
#include <random>

#define SM (CLS / sizeof(double)) // for L1 data optimization

const int MATRIX_SIZE = 1000;
double matrix1[MATRIX_SIZE][MATRIX_SIZE];
double matrix2[MATRIX_SIZE][MATRIX_SIZE];
double result[MATRIX_SIZE][MATRIX_SIZE];

double correct_result[MATRIX_SIZE][MATRIX_SIZE];

bool checkAnswer() {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      // 比较 double 不能直接用 ==
      if (abs(result[i][j] - correct_result[i][j]) > 1e-6) {
        std::cout << "Wrong answer: " << result[i][j]
                  << " != " << correct_result[i][j] << " at [" << i << ", " << j
                  << "]" << std::endl;
        return false;
      }
    }
  }
  std::cout << "Correct answer" << std::endl;
  return true;
}

void clearResult() {
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      result[i][j] = 0.0;
    }
  }
}

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

void multiple_no_opt() {
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      for (int k = 0; k < MATRIX_SIZE; ++k) {
        result[i][j] += matrix1[i][k] * matrix2[k][j];
      }
      correct_result[i][j] = result[i][j];
    }
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "No opt: " << diff.count() << " seconds"
            << std::endl; // 5.14706s
  clearResult();
}

void multiple_transpose_opt() {
  // 栈上分配的矩阵是一行一行连续存储的
  // 未优化版本中，matrix2[k][j] 显然没有访问连续的内存
  // 将其转置后就是连续的内存了
  auto transpose_start = std::chrono::steady_clock::now();
  double transposed_matrix2[MATRIX_SIZE][MATRIX_SIZE];
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      transposed_matrix2[i][j] = matrix2[j][i];
    }
  }
  auto transpose_end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = transpose_end - transpose_start;
  std::cout << "transpose cost: " << diff.count() << " seconds"
            << std::endl; // 0.0048

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
  checkAnswer();
  clearResult();
}

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
          // 重置 slice_mat2 指针
          slice_mat2 = &matrix2[k][j];
        }
      }
    }
  }
  auto multiple_end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = multiple_end - multiple_start;
  std::cout << "L1 data opt multiple cost: " << diff.count() << " seconds"
            << std::endl; // 3.4747
  checkAnswer();
  clearResult();
}

int main() {
  initMat();
  multiple_no_opt();
  multiple_transpose_opt();
  multiple_L1d_opt();
}
