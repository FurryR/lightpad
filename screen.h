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
  void output(bool print_prefix) const {
    if (print_prefix) {
      std::cout << prefix << (content == 0 ? ' ' : content);
    } else {
      std::cout << (content == 0 ? ' ' : content);
    }
  }
  bool operator!=(const Character& rhs) const noexcept {
    return rhs.prefix != prefix || ((content == 0 ? ' ' : content) !=
                                    (rhs.content == 0 ? ' ' : rhs.content));
  }
} Character;
typedef class Screen {
  std::vector<std::vector<Character>> current;
  std::vector<std::vector<Character>> buffer;
  Coord _size;
  bool dirty;
  void _init() const { std::cout << "\x1b[2J"; }
  bool _test(const Coord& pos) const noexcept {
    return pos.x < _size.x && pos.y < _size.y;
  }

 public:
  Screen(const Coord& size)
      : current(std::vector<std::vector<Character>>(
            size.y, std::vector<Character>(size.x))),
        buffer(std::vector<std::vector<Character>>(
            size.y, std::vector<Character>(size.x))),
        _size(size),
        dirty(false) {
    _init();
  }
  void show() {
    if (!dirty) return;
    bool flag = true;
    std::string str = "";
    for (size_t y = 0; y < _size.y; y++) {
      for (size_t x = 0; x < _size.x; x++) {
        if (current[y][x] != buffer[y][x]) {
          if (flag) {
            std::cout << "\x1b[" << (y + 1) << ";" << (x + 1) << "H";
            flag = false;
          }
          if (str != current[y][x].prefix) {
            std::cout << "\x1b[0m";
            str = current[y][x].prefix;
            current[y][x].output(true);
          } else {
            current[y][x].output(false);
          }
          buffer[y][x] = current[y][x];
        } else {
          flag = true;
        }
      }
    }
    dirty = false;
    std::cout << "\x1b[" << _size.y << ";" << _size.x << "H\x1b[0m"
              << std::flush;
  }
  void clear() {
    dirty = true;
    for (size_t y = 0; y < _size.y; y++) {
      for (size_t x = 0; x < _size.x; x++) {
        current[y][x] = Character();
      }
    }
  }
  bool set(const Coord& pos, const Character& chr) {
    if (!_test(pos)) return false;
    if (current[pos.y][pos.x] != chr) {
      dirty = true;
      current[pos.y][pos.x] = chr;
    }
    return true;
  }
  const Coord& size() const noexcept { return _size; }
} Screen;
#endif
