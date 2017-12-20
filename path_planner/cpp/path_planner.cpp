#include "path_planner.h"
#include <cmath>
#include <iostream>
#include <queue>

// for parallel-fsm
#include <thread>
#include "threadpool/ThreadPool.h"

// 找到某个维度更加接近原点的时间
double PathPlanner::findMinTime(const GridCell *c1, const GridCell *c2) {
  if (c1 == nullptr && c2 == nullptr) {
    // 两边都不存在
    return INF;
  }
  if (c1 == nullptr) {
    return c2->arrivalTime();
  }
  if (c2 == nullptr) {
    return c1->arrivalTime();
  }
  double t1 = c1->arrivalTime();
  double t2 = c2->arrivalTime();
  return t1 < t2 ? t1 : t2;
}

// 更新时间矩阵，并判断是否应该加入 Trial，如果是的话返回 true
// 所有状态相关的代码只跟fmm有关
bool PathPlanner::updateArrivalTime(GridMap &gm, GridCell *c) {
  if (c == nullptr || c->state() == Known) {
    return false;
  }

  bool retFlag = false;
  if (c->state() == Far) {
    c->setState(Trial);
    retFlag = true;
  }

  double cur_velocity = c->velocity();
  if (isZero(cur_velocity)) {
    c->setArrivalTime(INF);
    return retFlag;
  }

  int row = c->index() / gm.cols();
  int col = c->index() - row * gm.cols();
  GridCell *upCell = gm.getCellByRowCol(row - 1, col);
  GridCell *downCell = gm.getCellByRowCol(row + 1, col);
  GridCell *leftCell = gm.getCellByRowCol(row, col - 1);
  GridCell *rightCell = gm.getCellByRowCol(row, col + 1);
  // 找到各个维度领域的最小时间
  double verMinT = findMinTime(upCell, downCell);
  double horMinT = findMinTime(leftCell, rightCell);

  double newTime = INF;
  if (std::abs(verMinT - horMinT) >= 1.0 / cur_velocity) {
    newTime = std::min(verMinT, horMinT) + 1.0 / cur_velocity;
  } else {
    newTime = 0.5 * (verMinT + horMinT +
                     sqrt(2.0 / cur_velocity / cur_velocity -
                          (verMinT - horMinT) * (verMinT - horMinT)));
  }
  // printf("cell in [%d, %d], old time = %.3f, new time = %.3f\n", row, col,
  // c->getArrivalTime(), newTime);
  if (newTime < c->arrivalTime()) {
    c->setArrivalTime(newTime);
  }
  return retFlag;
}

// for fmm，寻找邻居
std::vector<GridCell *> PathPlanner::findNeighbour(GridMap &gm, GridCell *c) {
  std::vector<GridCell *> ret;
  int row = c->index() / gm.cols();
  int col = c->index() - row * gm.cols();
  GridCell *upCell = gm.getCellByRowCol(row - 1, col);
  GridCell *downCell = gm.getCellByRowCol(row + 1, col);
  GridCell *leftCell = gm.getCellByRowCol(row, col - 1);
  GridCell *rightCell = gm.getCellByRowCol(row, col + 1);
  if (upCell != nullptr) {
    ret.push_back(upCell);
  }
  if (downCell != nullptr) {
    ret.push_back(downCell);
  }
  if (leftCell != nullptr) {
    ret.push_back(leftCell);
  }
  if (rightCell != nullptr) {
    ret.push_back(rightCell);
  }
  return ret;
}

// squareXXX方法，没有重新拷贝 map
void PathPlanner::squaredXXX(GridMap &gm,
                             std::function<void(GridMap &)> method) {
  // 用于记录原来起点
  std::vector<GridCell *> startCells;

  // 将障碍物设为起点，起点设为普通点
  for (int i = 0; i < gm.cellNum(); i++) {
    GridCell *curCell = gm.getCellByIndex(i);
    if (curCell->arrivalTime() != INF) {
      // 找到起点
      curCell->setArrivalTime(INF);
      startCells.push_back(curCell);
      // 保持起点状态为Far,第二次扫描再更改
    }
    if (isZero(curCell->velocity())) {
      // 障碍点
      curCell->setArrivalTime(0.0);
      curCell->setVelocity(1.0);
    }
  }
  // 第一次运行 XXX
  method(gm);

  double maxArrivalTime = -1.0;
  // 恢复障碍物，同时找到最大的 arrivalTime，方便进行归一化
  for (int i = 0; i < gm.cellNum(); i++) {
    GridCell *curCell = gm.getCellByIndex(i);
    // 重点：恢复状态，对fmm非常关键
    curCell->setState(Far);
    double t = curCell->arrivalTime();
    if (t > maxArrivalTime) {
      maxArrivalTime = t;
    }
    if (isZero(t)) {
      // 此时到达时间为 0 的都是曾经的障碍点，恢复为障碍点
      curCell->setArrivalTime(INF);
      curCell->setVelocity(0.0);
    }
  }

  // 设置速度矩阵
  for (int i = 0; i < gm.cellNum(); i++) {
    GridCell *curCell = gm.getCellByIndex(i);
    if (isInf(curCell->arrivalTime())) {
      // 跳过障碍点
      continue;
    }
    // 设置速度，可以自行更改优化轨迹，但是要符合归一化的原则
    // curCell->setVelocity(std::sqrt(curCell->arrivalTime() / maxArrivalTime));
    curCell->setVelocity(curCell->arrivalTime() / maxArrivalTime);
    // 恢复 arrivalTime 为 INF，此时才算真正恢复了地图
    curCell->setArrivalTime(INF);
  }

  // 最后再恢复起点
  for (auto &pCell : startCells) {
    pCell->setArrivalTime(0);
  }
  // 再次运行 XXX
  method(gm);
}

// <某维度梯度值，能否沿着这个维度移动>
std::pair<double, bool>
PathPlanner::gradInOneDimension(double val1, double selfVal, double val2) {
  if (!isInf(val1) && !isInf(val2)) {
    return std::make_pair((val2 - val1) / 2.0, true);
  }
  double ret = 0.0;
  bool isValidDimension = true;
  if (isInf(val1) == true) {
    // | inf | x | val2 | 情况
    ret = val2 - selfVal;
    // 如果 ret > 0，那么希望撞墙走，非法，因此要变为 0.0 确保不撞墙
    if (ret > 0) {
      ret = 0.0;
      isValidDimension = false;
    }
  } else if (isInf(val2) == true) {
    // | val1 | x | inf | 情况
    ret = selfVal - val1;
    // 如果 ret < 0，那么希望撞墙走，非法，因此要变为 0.0 确保不撞墙
    if (ret < 0) {
      ret = 0.0;
      isValidDimension = false;
    }
  } else {
    // | inf | x | inf | 情况
    ret = 0.0;
  }
  return std::make_pair(ret, isValidDimension);
}

// 梯度下降算法求取路径
std::vector<std::pair<int, int>>
PathPlanner::gradient_descent(GridMap &gm, GridCell *startCell, int stepSize) {
  std::vector<std::pair<int, int>> ret;
  GridCell *curCell = startCell;
  while (!isZero(curCell->arrivalTime())) {
    // 还未到终点
    // 继续寻找 arrivalTime 更小的点
    int curRow = curCell->index() / gm.cols();
    int curCol = curCell->index() - gm.cols() * curRow;
    ret.push_back(std::make_pair(curRow, curCol));

    // 获取每个邻接点的 arrivalTime
    double upT = gm.getCellByRowCol(curRow - 1, curCol) == nullptr
                     ? INF
                     : gm.getCellByRowCol(curRow - 1, curCol)->arrivalTime();
    double downT = gm.getCellByRowCol(curRow + 1, curCol) == nullptr
                       ? INF
                       : gm.getCellByRowCol(curRow + 1, curCol)->arrivalTime();
    double leftT = gm.getCellByRowCol(curRow, curCol - 1) == nullptr
                       ? INF
                       : gm.getCellByRowCol(curRow, curCol - 1)->arrivalTime();
    double rightT = gm.getCellByRowCol(curRow, curCol + 1) == nullptr
                        ? INF
                        : gm.getCellByRowCol(curRow, curCol + 1)->arrivalTime();
    auto gradRowPair = gradInOneDimension(upT, curCell->arrivalTime(), downT);
    auto gradColPair =
        gradInOneDimension(leftT, curCell->arrivalTime(), rightT);
    double gradRow = gradRowPair.first;
    double gradCol = gradColPair.first;
    if (isZero(gradRow) && isZero(gradCol)) {
      //说明两个维度grad都是0，需判断以下特殊情况
      //      91
      //   89 90 89
      //     inf
      if (gradRowPair.second == gradColPair.second) {
        // 两个维度都不可行，或者两个维度都可行。只有一种情况，说明已经到终点
        // 已经在 while 循环中排除这种情况
        //      inf              1
        // inf  0   1       1   0   1
        //      1              1
        fprintf(stderr, "Error: bug point at cell (%d, %d): \n", curRow,
                curCol);
        std::cerr << *curCell << std::endl;
      } else if (gradRowPair.second == false) {
        // 随意设置往 Col 的任一方向走
        gradRow = 0;
        gradCol = 1;
      } else if (gradColPair.second == false) {
        gradRow = 1;
        gradCol = 0;
      } else {
        // never goes here
      }
    }

    // 归一化
    // double scale = std::max(std::abs(gradRow), std::abs(gradCol));
    double scale = std::sqrt(gradRow * gradRow + gradCol * gradCol);
    gradRow /= scale;
    gradCol /= scale;

    int nextRow = std::round(curRow - stepSize * gradRow);
    int nextCol = std::round(curCol - stepSize * gradCol);

    curCell = gm.getCellByRowCol(nextRow, nextCol);
    if (curCell == nullptr || (curRow == nextRow && curCol == nextCol)) {
      // 终止条件!!!
      // 1.越过地图边界
      // 2.仍是原来的点，说明已经无法找到更好的点
      break;
    }
    // ret.push_back(std::make_pair(nextRow, nextCol));
  }
  // 判断最后一个点是否是终点，如果不是将终点追加到末尾
  // std::pair 可以直接 == 判断
  if (ret.back() != gm.destination()) {
    ret.push_back(gm.destination());
  }
  return ret;
}

//------------------------------- APIs --------------------------------------
// fsm
void PathPlanner::fsm(GridMap &gm, int limCount) {
  std::cout << "Using fsm" << std::endl;
  int curCount = 0;
  while (curCount < limCount) {
    bool changed = false;
    ++curCount;
    for (int k = 1; k <= 4; k++) {
      int iStart;
      int jStart;
      int iStep;
      int jStep;
      switch (k) {
      case 1:
        // std::cout << "Sweeping quadrant-1 ..." << std::endl;
        iStart = 0;
        jStart = 0;
        iStep = 1;
        jStep = 1;
        break;
      case 2:
        // std::cout << "Sweeping quadrant-2 ..." << std::endl;
        iStart = gm.rows() - 1;
        jStart = 0;
        iStep = -1;
        jStep = 1;
        break;
      case 3:
        // std::cout << "Sweeping quadrant-3 ..." << std::endl;
        iStart = gm.rows() - 1;
        jStart = gm.cols() - 1;
        iStep = -1;
        jStep = -1;
        break;
      case 4:
        // std::cout << "Sweeping quadrant-4 ..." << std::endl;
        iStart = 0;
        jStart = gm.cols() - 1;
        iStep = 1;
        jStep = -1;
        break;
      }
      for (int i = iStart; 0 <= i && i < gm.rows(); i += iStep) {
        for (int j = jStart; 0 <= j && j < gm.cols(); j += jStep) {
          GridCell *curCell = gm.getCellByRowCol(i, j);
          double old_arrivalTime = curCell->arrivalTime();
          updateArrivalTime(gm, curCell);
          double new_arrivalTime = curCell->arrivalTime();
          if (isInf(old_arrivalTime) && isInf(new_arrivalTime)) {

          } else if (isInf(old_arrivalTime)) {
            //
            changed = true;
          } else if (old_arrivalTime / new_arrivalTime >= 1.01) {
            changed = true;
          } else {
            // never goes here
          }
        }
      }
    }
    if (changed == false) {
      // 打破 while 循环
      printf("fsm perfectly finished, sweep count = (%d / %d)\n", curCount,
             limCount);
      break;
    }
  }
}

// paralled fsm
void PathPlanner::pfsm(GridMap &gm, int limCount) {
  while (limCount) {
    limCount--;
    int diagNum = gm.cols() + gm.rows() - 1;
    int lv_start, lv_end, lv_step;

    // 右下
    lv_start = 0;
    lv_end = diagNum - 1;
    lv_step = 1;
    for (int level = lv_start; level <= lv_end; level += lv_step) {
      // i 的遍历方向与结果无关，主要是 level 的遍历方向
      int i_start = std::max(0, level - gm.cols() + 1);
      int i_end = std::min(gm.rows() - 1, level);
      for (int i = i_start; i <= i_end; i++) {
        int j = level - i;
        updateArrivalTime(gm, gm.getCellByRowCol(i, j));
      }
    }

    // 右上
    lv_start = gm.rows() - 1;
    lv_end = -(gm.cols() - 1);
    lv_step = -1;
    for (int level = lv_start; level >= lv_end; level += lv_step) {
      int i_start = std::max(0, level);
      int i_end = std::min(gm.rows() - 1, gm.rows() + level);
      for (int i = i_start; i <= i_end; i++) {
        int j = i - level;
        updateArrivalTime(gm, gm.getCellByRowCol(i, j));
      }
    }

    // 左上
    lv_start = diagNum - 1;
    lv_end = 0;
    lv_step = -1;
    for (int level = lv_start; level >= lv_end; level += lv_step) {
      int i_start = std::max(0, level - gm.cols() + 1);
      int i_end = std::min(gm.rows() - 1, level);
      for (int i = i_start; i <= i_end; i++) {
        int j = level - i;
        updateArrivalTime(gm, gm.getCellByRowCol(i, j));
      }
    }

    // 左下
    lv_start = -(gm.cols() - 1);
    lv_end = gm.rows() - 1;
    lv_step = 1;
    for (int level = lv_start; level <= lv_end; level += lv_step) {
      int i_start = std::max(0, level);
      int i_end = std::min(gm.rows() - 1, gm.rows() + level);
      for (int i = i_start; i <= i_end; i++) {
        int j = i - level;
        updateArrivalTime(gm, gm.getCellByRowCol(i, j));
      }
    }
  }
}

// paralled fsm
void PathPlanner::parallel_fsm(GridMap &gm, int limCount) {

  const int threadNum = std::thread::hardware_concurrency();

  int thresh = 200; // 大于 200 则开始并行，避免线程开销

  ThreadPool thread_pool(threadNum);

  while (limCount) {
    limCount--;
    int diagNum = gm.cols() + gm.rows() - 1;
    int lv_start, lv_end, lv_step;

    // 右下
    lv_start = 0;
    lv_end = diagNum - 1;
    lv_step = 1;
    for (int level = lv_start; level <= lv_end; level += lv_step) {
      // i 的遍历方向与结果无关，主要是 level 的遍历方向
      int i_start = std::max(0, level - gm.cols() + 1);
      int i_end = std::min(gm.rows() - 1, level);
      auto task = [&](int offset, int interval) {
        for (int i = i_start + offset; i <= i_end; i += interval) {
          int j = level - i;
          updateArrivalTime(gm, gm.getCellByRowCol(i, j));
        }
      };
      if (i_end - i_start > thresh) {
        for (int i = 0; i < threadNum; i++) {
          thread_pool.AddJob([=](){
            task(i, threadNum);
          });
        }
        thread_pool.WaitAll();
      } else {
        task(0, 1);
      }
    }

    // 右上
    lv_start = gm.rows() - 1;
    lv_end = -(gm.cols() - 1);
    lv_step = -1;
    for (int level = lv_start; level >= lv_end; level += lv_step) {
      int i_start = std::max(0, level);
      int i_end = std::min(gm.rows() - 1, gm.rows() + level);
      auto task = [&](int offset, int interval) {
        for (int i = i_start + offset; i <= i_end; i += interval) {
          int j = i - level;
          updateArrivalTime(gm, gm.getCellByRowCol(i, j));
        }
      };
      if (i_end - i_start > thresh) {
        for (int i = 0; i < threadNum; i++) {
          thread_pool.AddJob([=](){
            task(i, threadNum);
          });
        }
        thread_pool.WaitAll();
      } else {
        task(0, 1);
      }
    }

    // 左上
    lv_start = diagNum - 1;
    lv_end = 0;
    lv_step = -1;
    for (int level = lv_start; level >= lv_end; level += lv_step) {
      int i_start = std::max(0, level - gm.cols() + 1);
      int i_end = std::min(gm.rows() - 1, level);
      auto task = [&](int offset, int interval) {
        for (int i = i_start + offset; i <= i_end; i += interval) {
          int j = level - i;
          updateArrivalTime(gm, gm.getCellByRowCol(i, j));
        }
      };
      if (i_end - i_start > thresh) {
        for (int i = 0; i < threadNum; i++) {
          thread_pool.AddJob([=](){
            task(i, threadNum);
          });
        }
        thread_pool.WaitAll();
      } else {
        task(0, 1);
      }
    }

    // 左下
    lv_start = -(gm.cols() - 1);
    lv_end = gm.rows() - 1;
    lv_step = 1;
    for (int level = lv_start; level <= lv_end; level += lv_step) {
      int i_start = std::max(0, level);
      int i_end = std::min(gm.rows() - 1, gm.rows() + level);
      auto task = [&](int offset, int interval) {
        for (int i = i_start + offset; i <= i_end; i += interval) {
          int j = i - level;
          updateArrivalTime(gm, gm.getCellByRowCol(i, j));
        }
      };
      if (i_end - i_start > thresh) {
        for (int i = 0; i < threadNum; i++) {
          thread_pool.AddJob([=](){
            task(i, threadNum);
          });
        }
        thread_pool.WaitAll();
      } else {
        task(0, 1);
      }
    }
  }
}

// squared fsm
void PathPlanner::sfsm(GridMap &gm, int limCount) {
  std::cout << "Using sfsm" << std::endl;
  auto wrapped_fsm = std::bind(fsm, std::placeholders::_1, limCount);
  squaredXXX(gm, wrapped_fsm);
}

void PathPlanner::fmm(GridMap &gm) {
  std::cout << "Using fmm" << std::endl;
  auto cmp = [](const GridCell *c1, const GridCell *c2) {
    // 大于生成小顶堆
    return c1->arrivalTime() > c2->arrivalTime();
  };
  std::priority_queue<GridCell *, std::vector<GridCell *>, decltype(cmp)> pq(
      cmp);
  // 遍历寻找起点，加入Trial优先队列
  for (int i = 0; i < gm.rows(); i++) {
    for (int j = 0; j < gm.cols(); j++) {
      GridCell *curCell = gm.getCellByRowCol(i, j);
      if (curCell->arrivalTime() != INF) {
        curCell->setState(Trial);
        pq.push(curCell);
      }
    }
  }
  while (!pq.empty()) {
    // std::cout << pq.size() << " cells in Trial" << std::endl;

    // 为了绘图，运行到一半中止
    // if (iter_count_ > 319200* 0.6) {
    //   break;
    // }

    GridCell *curCell = pq.top();
    pq.pop();
    curCell->setState(Known);

    // std::cout << "Now process: [" << row << ", " << col <<"]" << std::endl;
    auto neighbours = findNeighbour(gm, curCell);
    for (auto &neighbour : neighbours) {
      if (updateArrivalTime(gm, neighbour) == true) {
        pq.push(neighbour);
      }
    }
  }
}

// squared fmm
void PathPlanner::sfmm(GridMap &gm) {
  std::cout << "Using sfmm" << std::endl;
  squaredXXX(gm, fmm);
}

std::vector<std::pair<int, int>>
PathPlanner::getPath(GridMap &gm, int startRow, int startCol, int stepSize) {
  std::vector<std::pair<int, int>> ret;
  GridCell *startCell = gm.getCellByRowCol(startRow, startCol);
  if (startCell == nullptr || isZero(startCell->velocity())) {
    fprintf(stderr, "Error: set start at obstacle (%d, %d)\n", startRow,
            startCol);
    return ret;
  }
  ret = gradient_descent(gm, startCell, stepSize);
  return ret;
}
