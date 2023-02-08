#ifndef _UTILS_H
#define _UTILS_H
#include <termios.h>
#include <unistd.h>

#include <iostream>

#include "./awacorn/awacorn.h"
#include "./awacorn/promise.h"
#include "./screen.h"
/**
 * @brief 判断输入流中是否还有字符。此API用于补足kbhit的缺失。
 *
 * @return int 0（未获得输入），1（获得输入）。
 */
int kbhit() {
  struct termios oldt, newt;
  // int ch, oldf;
  char ch = 0;
  int readbytes = 0;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  readbytes = read(STDIN_FILENO, &ch, 1);
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  if (readbytes) {
    ungetc(ch, stdin);
    return 1;
  }
  return 0;
}
int getch() {
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}
Coord getsize() {
  Coord ret = Coord(0, 0);
  std::cout << "\x1b[s\x1b[9999;9999H\x1b[6n\x1b[u";
  getch();
  getch();
  for (char ch; (ch = getch()) != ';'; ret.y = ret.y * 10 + (ch - '0'))
    ;
  for (char ch; (ch = getch()) != 'R'; ret.x = ret.x * 10 + (ch - '0'))
    ;
  return ret;
}
Awacorn::AsyncFn<Promise::Promise<int>> async_getch() {
  return [](Awacorn::EventLoop* ev) {
    Promise::Promise<int> pm;
    ev->create(
        [pm](Awacorn::EventLoop* ev, const Awacorn::Interval* fn) -> void {
          if (kbhit()) {
            pm.resolve(getch());
            ev->clear(fn);
          }
        },
        std::chrono::milliseconds(10));
    return pm;
  };
}
#endif