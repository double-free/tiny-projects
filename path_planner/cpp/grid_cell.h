#pragma once
#include <iostream>
#include <cfloat>
#include <cmath>
const double INF = DBL_MAX;

enum CellState {
  Known,
  Trial,
  Far
};

inline bool isZero(double f) {
  if (std::abs(f) < 1e-10) {
    return true;
  }
  return false;
}

inline bool isInf(double f) {
  if (std::abs(INF - f) < 1e-10) {
    return true;
  }
  return false;
}

class GridCell {
public:
  GridCell():index_(-1), arrivalTime_(INF), velocity_(1.0), state_(Far){}
  explicit GridCell(int idx, double vel)
    :index_(idx), arrivalTime_(INF), velocity_(1.0), state_(Far) {}

  double velocity() const {
    return velocity_;
  }
  void setVelocity(double vel) {
    velocity_ = vel;
  }

  double arrivalTime() const {
    return arrivalTime_;
  }
  void setArrivalTime(double t) {
    arrivalTime_ = t;
  }

  int index() const {
    return index_;
  }
  void setIndex(int idx) {
    index_ = idx;
  }

  friend std::ostream& operator<<(std::ostream& out, const GridCell& c) {
    out << "Basic cell information: " << '\n'
                 <<'\t' << "Index: " << c.index() << '\n'
                   <<'\t'<<"Arrival Time:" <<c.arrivalTime()<<'\n'
                     <<'\t'<<"Velocity:" <<c.velocity()<<'\n'
                    <<'\t' <<"Address:" <<&c;
    return out;
  }

  CellState state() const {
    return state_;
  }
  void setState(CellState s) {
    state_ = s;
  }

private:
  int index_;
  double arrivalTime_;
  double velocity_;
  // only for fmm
  // Far: 初始化为 inf 的点
  // Trial: 从 Far 更新后成为 Trial
  // Known: 从 Trial 更新，不再需要更新的点
  CellState state_;
};
