#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "path_planner.h"

#include "inih/cpp/INIReader.h"

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    printf("Path Planning for 2D map!\n"
           "\tUsage: %s <*.ini>\n",
           argv[0]);
    return -1;
  }
  // 检查是不是ini文件
  std::string cfgFile(argv[1]);
  if (cfgFile.substr(cfgFile.size() - 4) != ".ini") {
    fprintf(stderr, "%s is not a configuration file\n", argv[1]);
    return -1;
  }

  INIReader cfg(cfgFile);
  if (cfg.ParseError() < 0) {
    fprintf(stderr, "Can not parse configuration file: %s\n", argv[1]);
    return -1;
  }

  std::string inputFile = cfg.Get("IO", "inputFile", "");
  std::ifstream ifs(inputFile);
  std::string line;
  // 输入是只含 0，1 的 .csv，代表二值图
  std::vector<std::string> tmp;
  while (std::getline(ifs, line)) {
    tmp.push_back(line);
  }
  ifs.clear();
  ifs.close();

  if (tmp.size() == 0) {
    return 0;
  }
  int rowNum = tmp.size();
  int colNum = (tmp[0].size() + 1) / 2;
  GridMap gm(rowNum, colNum);
  for (int i = 0; i < rowNum; i++) {
    for (int j = 0; j < colNum; j++) {
      int idx = i * colNum + j;
      GridCell *curCell = gm.getCellByRowCol(i, j);
      curCell->setIndex(idx);
      char c = tmp[i][2 * j];
      if (c == '0') {
        //障碍物
        curCell->setVelocity(0);
      }
    }
  }

  // 如果需要远离边界
  if (cfg.GetBoolean("Parameters", "awayFromBoundary", false) == true) {
    // 设置地图周围一圈为障碍物
    gm.setBoundary();
  }

  // 先设置边界再设置终点
  int endRow = cfg.GetInteger("Parameters", "endRow", -1);
  int endCol = cfg.GetInteger("Parameters", "endCol", -1);
  if (gm.setDestination(endRow, endCol) < 0) {
    fprintf(stderr, "Error: set destination at obstacle: (%d, %d),\n", endRow,
            endCol);
    return -1;
  }

  // 计算时间矩阵
  int limCount = cfg.GetInteger("Parameters", "fsmSweepCountLim", 100);
  int thresh = cfg.GetInteger("Parameters", "parallelThresh", 1000);
  auto start = std::chrono::steady_clock::now();
  std::string method = cfg.Get("Parameters", "method", "");
  if (method == "fsm") {
    PathPlanner::fsm(gm, limCount);
  } else if (method == "pfsm") {
    PathPlanner::pfsm(gm, limCount);
  } else if (method == "parallel_fsm") {
    PathPlanner::parallel_fsm(gm, limCount, thresh);
  } else if (method == "sfsm") {
    PathPlanner::sfsm(gm, limCount);
  } else if (method == "fmm") {
    PathPlanner::fmm(gm);
  } else if (method == "sfmm") {
    PathPlanner::sfmm(gm);
  } else {
    std::cout << "Unkown method: " << method << std::endl;
    return -1;
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << method << " cost " << diff.count()
            << " seconds " << std::endl;

  // 计算路径
  int startRow = cfg.GetInteger("Parameters", "startRow", -1);
  int startCol = cfg.GetInteger("Parameters", "startCol", -1);
  int stepSize = cfg.GetInteger("Parameters", "stepSize", 1);
  std::vector<std::pair<int, int>> path =
      PathPlanner::getPath(gm, startRow, startCol, stepSize);

  // 保存结果
  bool shouldSaveResult = cfg.GetBoolean("IO", "saveResult", false);
  if (shouldSaveResult) {
    std::string outputDir = cfg.Get("IO", "outputDir", "./");
    std::string mapName;
    for (int i = inputFile.size() - 1; i >= 0; i--) {
      if (inputFile[i] == '/') {
        mapName = inputFile.substr(i + 1, inputFile.size() - i - 5);
        break;
      }
    }
    // 存储时间矩阵
    std::string tMatFile = outputDir + mapName + "TimeMat_" + method + ".csv";
    // std::cout <<tMatFile;
    std::ofstream ofs(tMatFile, std::ios::out);
    for (int i = 0; i < gm.rows(); i++) {
      for (int j = 0; j < gm.cols(); j++) {
        GridCell *curCell = gm.getCellByRowCol(i, j);
        double t = curCell->arrivalTime();
        if (isInf(t)) {
          // 区分不可达点和障碍物
          if (isZero(curCell->velocity())) {
            // 障碍物
            ofs << "Inf";
          } else {
            // 不可达点
            ofs << "NaN";
          }

          // 不区分
          // ofs << "inf";
        } else {
          ofs << t;
        }
        if (j != gm.cols() - 1) {
          ofs << ", ";
        }
      }
      ofs << "\n";
    }
    ofs.clear();
    ofs.close();
    std::cout << "Time matrix saved in " << tMatFile << std::endl;

    // 保存路径
    std::string pathFile = outputDir + mapName + "Path_" + method + ".csv";
    ofs.open(pathFile, std::ios::out);
    for (const auto &row_col : path) {
      ofs << row_col.first << ", " << row_col.second << "\n";
    }
    ofs.clear();
    ofs.close();
    std::cout << "Path saved in " << pathFile << std::endl;
    return 0;
  }
}
