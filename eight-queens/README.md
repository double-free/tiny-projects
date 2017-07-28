问题
---
[来自知乎](https://www.zhihu.com/question/62185153/answer/196193958?utm_medium=social&utm_source=wechat_session&from=groupmessage&isappinstalled=1)




实验结果
---
测试系统: OS X </br>
处理器: 2.7 GHz Intel Core i5 </br>
C++ 编译器: Apple LLVM version 8.0.0 (clang-800.0.42.1) </br>
Golang 版本: go1.8.1 darwin/amd64 </br>
Python 版本: 3.5.1

### C++
编译器不优化：<br/>
- 棋盘大小13 </br>
Solution num: 73712 </br>
elapsed time = 0.504258 seconds

- 棋盘大小15 </br>
Solution num: 2279184 </br>
elapsed time = 19.6999 seconds

-O2 优化: <br/>
- 棋盘大小13 </br>
Solution num: 73712 </br>
elapsed time = 0.31887 seconds

- 棋盘大小15 </br>
Solution num: 2279184 </br>
elapsed time = 10.1004 seconds

### Golang
- 棋盘大小13 </br>
Solution num: 73712 </br>
elapsed time = 0.351473 seconds

- 棋盘大小15 </br>
Solution num: 2279184 </br>
elapsed time = 14.468555985 seconds

### Python
- 棋盘大小13 </br>
Solution num: 73712 </br>
elapsed time = 27.48484206199646 seconds


结论
---
开启编译器优化后 C/C++ 优势还是相当明显的。
