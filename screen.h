#ifndef _SCREEN_H
#define _SCREEN_H
#include <iostream>
#include <map>
#include <vector>
typedef struct Coord {
  size_t x, y;
  Coord() {}
  Coord(size_t x, size_t y) : x(x), y(y) {}
} Coord;
typedef struct Character {
  std::string prefix;
  char content;
  Character() : prefix(""), content(0) {}
  explicit Character(char content) : prefix(""), content(content) {}
  Character(char content, const std::string& prefix)
      : prefix(prefix), content(content) {}
  void output() const {
    if (prefix != "") {
      std::cout << prefix << (content == 0 ? ' ' : content);
    } else {
      std::cout << (content == 0 ? ' ' : content);
    }
  }
  bool operator==(const Character& rhs) const noexcept {
    return rhs.prefix == prefix && content == rhs.content;
  }
} Character;
typedef class Screen {
  std::vector<std::vector<Character>> buf;
  std::vector<std::vector<Character>> history;
  // std::vector<std::vector<Character>> history;
  Coord _size;
  void _init() const { std::cout << "\x1b[2J\x1b[1;1H"; }
  void _clear() const { std::cout << "\x1b[1;1H"; }
  bool _test(const Coord& pos) const noexcept {
    return pos.x < _size.x && pos.y < _size.y;
  }

 public:
  Screen(const Coord& size)
      : buf(std::vector<std::vector<Character>>(
            size.y, std::vector<Character>(size.x))),
        history(std::vector<std::vector<Character>>(
            size.y, std::vector<Character>(size.x))),
        _size(size) {
    _init();
  }
  void show() {
    bool flag = false;
    _clear();
    for (size_t y = 0; y < _size.y; y++) {
      for (size_t x = 0; x < _size.x; x++) {
        if (!(buf[y][x] == history[y][x])) {
          std::cout << "\x1b[0m";
          if (flag) {
            std::cout << "\x1b[" << (y + 1) << ";" << (x + 1) << "H";
            flag = false;
          }
          buf[y][x].output();
          history[y][x] = buf[y][x];
        } else {
          flag = true;
        }
      }
    }
    std::cout << "\x1b[" << _size.y << ";" << _size.x << "H\x1b[0m"
              << std::flush;
  }
  void clear() {
    for (size_t y = 0; y < _size.y; y++) {
      for (size_t x = 0; x < _size.x; x++) {
        buf[y][x] = Character();
      }
    }
  }
  bool set(const Coord& pos, const Character& chr) {
    if (!_test(pos)) return false;
    // history[pos] = buf[pos.y][pos.x];
    buf[pos.y][pos.x] = chr;
    return true;
  }
  const Coord& size() const noexcept { return _size; }
} Screen;
#endif