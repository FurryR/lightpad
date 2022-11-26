#ifndef _UTILS_H
#define _UTILS_H
#include <iostream>

#include "./screen.h"

#ifndef _WIN32
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
/**
 * @brief 判断输入流中是否还有字符。此API用于补足kbhit的缺失。
 *
 * @return int 0（未获得输入），1（获得输入）。
 */
int kbhit() {
  struct termios oldt, newt;
  int ch, oldf;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
  if (ch != EOF) {
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
#else
#include <conio.h>
#endif
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
#endif