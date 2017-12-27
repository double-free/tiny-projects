#pragma once
#include "grid_cell.h"
#include <vector>

class GridMap {
public:
  GridMap() {
    rows_ = 0;
    cols_ = 0;
    gridSize_ = 1.0;
  }
  explicit GridMap(int rows, int cols, double gridSize = 1.0):rows_(rows), cols_(cols), gridSize_(gridSize) {
    cells_.resize(rows*cols);
  }

  void resize(int rows, int cols, double gridSize = 1.0) {
    rows_ = rows;
    cols_ = cols;
    gridSize_ = gridSize;
    cells_.resize(rows*cols);
  }

  int rows() const {
    return rows_;
  }

  int cols() const {
    return cols_;
  }

  int cellNum() const {
    return cells_.size();
  }

  double gridSize() const {
    return gridSize_;
  }

  GridCell* getCellByIndex(int idx) {
    if (idx<0 || idx >= rows_*cols_) {
      // 越界返回空指针
      return nullptr;
    }
    return &cells_[idx];
  }

  GridCell* getCellByRowCol(int row, int col) {
    if (row<0 || row>=rows_ || col<0 || col>=cols_) {
      // 越界返回空指针
      return nullptr;
    }
    return &cells_[row*cols_ + col];
  }

  // 返回设置点的idx, 失败返回 -1
  int setDestination(int row, int col) {
    GridCell* curCell = getCellByRowCol(row, col);
    if (curCell == nullptr || isZero(curCell->velocity())) {
      return -1;
    }
    curCell->setArrivalTime(0);
    dstCell_ = curCell;
    return curCell->index();
  }

  // 设置边界为障碍物
  void setBoundary() {
    // 左右边界
    for (int i=0; i<rows_; i++) {
      getCellByRowCol(i, 0)->setVelocity(0.0);
      getCellByRowCol(i, cols_-1)->setVelocity(0.0);
    }
    // 上下边界
    for (int j=0; j<cols_; j++) {
      getCellByRowCol(0, j)->setVelocity(0.0);
      getCellByRowCol(rows_-1, j)->setVelocity(0.0);
    }
  }

  std::pair<int, int> destination() const {
    std::pair<int, int> ret{-1,-1};
    if (dstCell_ == nullptr) {
      return ret;
    }
    ret.first = dstCell_->index() / this->cols();
    ret.second = dstCell_->index() - this->cols()*ret.first;
    return ret;
  }
private:
  int rows_;
  int cols_;
  double gridSize_;
  std::vector<GridCell> cells_;
  GridCell* dstCell_;
};
