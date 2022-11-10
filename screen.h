#ifndef _SCREEN_H
#define _SCREEN_H
#include <iostream>
#include <memory>
#include <vector>
typedef struct Coord {
  size_t x, y;
  Coord() {}
  Coord(size_t x, size_t y) : x(x), y(y) {}
} Coord;
typedef struct Character {
  std::shared_ptr<std::string> prefix;
  char content;
  Character() : prefix(nullptr), content(0) {}
  explicit Character(char content) : content(content), prefix(nullptr) {}
  Character(char content, const std::string& prefix)
      : content(content), prefix(new std::string(prefix)) {}
  void output(const Character* last) const {
    if (prefix) {
      if (content == 0) {
        if (last && last->prefix && *last->prefix != *prefix)
          std::cout << "\033[0m";
        std::cout << (*prefix) << ' ';
      } else {
        if (last && last->prefix && *last->prefix != *prefix)
          std::cout << "\033[0m";
        std::cout << (*prefix) << content;
      }
    } else {
      if (last && last->prefix) std::cout << "\033[0m";
      if (content != 0)
        std::cout << content;
      else
        std::cout << ' ';
    }
  }
} Character;
typedef class Screen {
  std::vector<std::vector<Character>> buf;
  Coord _size;
  void _init() const { std::cout << "\e[2J\e[0;0H"; }
  void _clear() const { std::cout << "\e[0;0H"; }
  bool _test(const Coord& pos) const noexcept {
    return pos.x < _size.x && pos.y < _size.y;
  }

 public:
  Screen(const Coord& size)
      : _size(size),
        buf(std::vector<std::vector<Character>>(
            size.y, std::vector<Character>(size.x))) {
    _init();
  }
  void show() const {
    _clear();
    const Character* last = nullptr;
    for (size_t y = 0; y < _size.y; y++) {
      for (size_t x = 0; x < _size.x; x++) {
        buf[y][x].output(last);
        last = &(buf[y][x]);
      }
      if (y < _size.y - 1) std::cout << std::endl;
    }
    std::cout << "\033[0m" << std::flush;
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
    buf[pos.y][pos.x] = chr;
    return true;
  }
  const Coord& size() const noexcept { return _size; }
} Screen;
#endif