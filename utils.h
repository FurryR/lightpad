#ifndef _UTILS_H
#define _UTILS_H
#include <iostream>
#include "./screen.h"

#ifndef _WIN32
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
/**
 * @brief 非阻塞getch IO。
 *
 * @return int -1（未获得输入），或char code。
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
    return ch;
  }
  return -1;
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
#include <termios.h>
Coord getsize() {
  Coord ret = Coord(0, 0);
  termios tm, tm_old;
  tcgetattr(0, &tm_old);
  tm = tm_old;
  tm.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(0, TCSANOW, &tm);
  std::cout << "\033[s\033[9999;9999H\033[6n\033[u";
  getchar();
  getchar();
  for (char ch; (ch = getchar()) != ';'; ret.y = ret.y * 10 + (ch - '0'))
    ;
  for (char ch; (ch = getchar()) != 'R'; ret.x = ret.x * 10 + (ch - '0'))
    ;
  tcsetattr(0, TCSANOW, &tm_old);
  return ret;
}
#else
#include <conio.h>
Coord getsize() {
  Coord ret = Coord(0, 0);
  std::cout << "\033[s\033[9999;9999H\033[6n\033[u";
  getch();
  getch();
  for (char ch; (ch = getch()) != ';'; ret.y = ret.y * 10 + (ch - '0'))
    ;
  for (char ch; (ch = getch()) != 'R'; ret.x = ret.x * 10 + (ch - '0'))
    ;
  return ret;
}
#endif
#endif