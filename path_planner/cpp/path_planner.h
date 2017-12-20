#pragma once
#include "grid_map.h"
#include <functional>
#include <atomic>

class PathPlanner {
private:
  // 找到某个维度更加接近原点的时间
  static double findMinTime(const GridCell *c1, const GridCell *c2);

  // 更新时间矩阵，并判断是否应该加入 Trial，如果是的话返回 true
  static bool updateArrivalTime(GridMap &gm, GridCell *c);

  // for fmm，寻找邻居
  static std::vector<GridCell *> findNeighbour(GridMap &gm, GridCell *c);

  // for squaredXXX，传入 fmm 或者 fsm 函数
  static void squaredXXX(GridMap& gm, std::function<void (GridMap&)> method);

  // 求出某一个维度的梯度
  static std::pair<double, bool> gradInOneDimension(double val1, double selfVal, double val2);
  // 梯度下降计算路径
  static std::vector<std::pair<int, int>> gradient_descent(GridMap& gm, GridCell* startCell, int stepSize = 1);

public:
  // fsm，需要指明扫描多少次，对复杂地形扫描次数不够会导致结果不正确
  static void fsm(GridMap &gm, int limCount);

  // pfsm，改变扫描方式，便于并行的 fsm
  static void pfsm(GridMap &gm, int limCount);

  // parallel_fsm 真正用多线程实现的 fsm
  static void parallel_fsm(GridMap &gm, int limCount);

  // squared fsm
  static void sfsm(GridMap &gm, int limCount);
  // fmm
  static void fmm(GridMap &gm);
  // squared fmm
  static void sfmm(GridMap &gm);
  // get path
  static std::vector<std::pair<int, int>> getPath(GridMap& gm, int startRow, int startCol, int stepSize = 1);

};
