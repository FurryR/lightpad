#include <algorithm>

#include "./render/tomorrow-night-bright-cpp.h"
#include "./screen.h"
#include "./utils.h"
#define TAB_SIZE 2
typedef struct Modestr {
  std::string prefix;
  std::string mode;
  Modestr(const std::string& mode, const std::string& prefix)
      : prefix(prefix), mode(mode) {}
} Modestr;
void show_text(Screen* screen, const std::vector<std::vector<Character>>& text,
               const Coord& cursor, size_t xoffset = 0, size_t yoffset = 0) {
  size_t vec_index = yoffset, str_index = 0;
  size_t y = 0, x = 0;
  const Coord& sz = screen->size();
  for (; y < sz.y - 2; y++) {
    str_index = xoffset;
    if (vec_index < text.size()) {
      for (x = 0; x < sz.x; x++) {
        if (str_index < text[vec_index].size()) {
          if (vec_index == cursor.y && str_index == cursor.x) {
            screen->set(Coord(x, y),
                        Character(text[vec_index][str_index].content,
                                  text[vec_index][str_index].prefix
                                      ? (*text[vec_index][str_index].prefix +
                                         "\033[47m")
                                      : "\033[47m"));
          } else {
            screen->set(Coord(x, y), text[vec_index][str_index]);
          }
          str_index++;
        } else
          break;
      }
      if (vec_index == cursor.y && str_index == cursor.x) {
        screen->set(Coord(x, y), Character(0, "\033[47m"));
      }
    } else
      break;
    vec_index++;
  }
  if (vec_index == cursor.y) {
    screen->set(Coord(0, y), Character(0, "\033[47m"));
  }
}
void show_bar(Screen* screen, const Modestr& mode, const std::string& hint,
              const std::string& back_str) {
  // 左侧
  screen->set(Coord(0, screen->size().y - 2), Character(0, mode.prefix));
  for (size_t i = 0; i < mode.mode.length(); i++) {
    screen->set(Coord(i + 1, screen->size().y - 2),
                Character(mode.mode[i], mode.prefix));
  }
  screen->set(Coord(mode.mode.length() + 1, screen->size().y - 2),
              Character(0, mode.prefix));
  // 中间
  size_t idx = 0;
  for (size_t i = mode.mode.length() + 2; i < screen->size().x; i++) {
    if (idx < hint.length() && i > mode.mode.length() + 2) {
      screen->set(Coord(i, screen->size().y - 2),
                  Character(hint[idx], "\033[97;44m"));
      idx++;
    } else {
      screen->set(Coord(i, screen->size().y - 2), Character(0, "\033[44m"));
    }
  }
  // 右侧
  screen->set(Coord(screen->size().x - 1, screen->size().y - 2),
              Character(0, "\033[43m"));
  for (size_t i = 0; i < back_str.length(); i++) {
    screen->set(Coord(screen->size().x - (back_str.length() - i) - 1,
                      screen->size().y - 2),
                Character(back_str[i], "\033[30;43m"));
  }
  screen->set(
      Coord(screen->size().x - back_str.length() - 2, screen->size().y - 2),
      Character(0, "\033[43m"));
}
void show_info(Screen* screen, const std::string& info) {
  for (size_t x = 0; x < screen->size().x; x++) {
    screen->set(Coord(x, screen->size().y - 1), Character(0));
  }
  for (size_t i = 0; i < info.length(); i++) {
    screen->set(Coord(i, screen->size().y - 1), Character(info[i]));
  }
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
typedef enum Mode { Normal = 0, Insert = 1 } Mode;
Modestr mode2str(Mode mode) {
  switch (mode) {
    case Normal:
      return Modestr("NORMAL", "\033[42m\033[30m");
    case Insert:
      return Modestr("INSERT", "\033[43m\033[30m");
  }
  return Modestr("UNKNOWN", "");
}
// 默认的 render。不会给代码上色。
std::vector<std::vector<Character>> render_plaintext(
    const std::vector<std::string>& str) {
  std::vector<std::vector<Character>> ret;
  for (size_t i = 0; i < str.size(); i++) {
    std::vector<Character> tmp;
    for (size_t j = 0; j < str[i].size(); j++) {
      tmp.push_back(Character(str[i][j]));
    }
    ret.push_back(tmp);
  }
  return ret;
}
void process_arrow(Coord* cur_pos, size_t* x_before,
                   const std::vector<std::string>& text, char op) {
  switch (op) {
    // up
    case 'A': {
      if (cur_pos->y > 0) {
        cur_pos->y--;
        if (*x_before >= text[cur_pos->y].length()) {
          cur_pos->x = text[cur_pos->y].length();
        } else {
          cur_pos->x = *x_before;
        }
      }
      break;
    }
    // down
    case 'B': {
      if (cur_pos->y < text.size() - 1) {
        cur_pos->y++;
        if (*x_before >= text[cur_pos->y].length()) {
          cur_pos->x = text[cur_pos->y].length();
        } else {
          cur_pos->x = *x_before;
        }
      }
      break;
    }
    // left
    case 'D': {
      if (cur_pos->x > 0)
        cur_pos->x--;
      else if (cur_pos->y > 0) {
        cur_pos->y--;
        cur_pos->x = text[cur_pos->y].size();
      }
      *x_before = cur_pos->x;
      break;
    }
    // right
    case 'C': {
      if (cur_pos->x < text[cur_pos->y].length())
        cur_pos->x++;
      else if (cur_pos->y < text.size() - 1) {
        cur_pos->y++;
        cur_pos->x = 0;
      }
      *x_before = cur_pos->x;
      break;
    }
  }
}
void main_ui(Screen* screen) {
  std::vector<std::string> text;
  text.push_back("#include <iostream>");
  text.push_back("// An example c++ program rendered by lightpad-cpprender.");
  text.push_back("int main() {");
  text.push_back("  std::cout << \"Hello World!\\n\" << std::endl;");
  text.push_back("  return 0;");
  text.push_back("}");
  Coord cur_pos = Coord(0, 0);
  size_t x_before = 0;
  Mode mode = Normal;
  int cmd = 0;
  while (1) {
    screen->clear();
    show_text(
        screen, TomorrowNightBrightCpp::render(text), cur_pos,
        (size_t)std::max<int>({0, int(cur_pos.x - screen->size().x)}),
        (size_t)std::max<int>({0, int(cur_pos.y - screen->size().y + 3)}));
    show_bar(screen, mode2str(mode), "[unnamed]",
             std::string("ln: ") + std::to_string(cur_pos.y + 1) + "/" +
                 std::to_string(text.size()) +
                 " col: " + std::to_string(cur_pos.x + 1));
    screen->show();
    while (1) {
      bool flag = true;
      cmd = getch();
      switch (cmd) {
        case '\e': {
          // Esc, 方向键
          if (kbhit() == '[') {
            // 方向键
            getch();
            process_arrow(&cur_pos, &x_before, text, getch());
          } else {
            // 切换为普通模式
            if (mode == Normal) {
              flag = false;
            } else
              mode = Normal;
          }
          break;
        }
        case '\n': {
          if (mode == Insert) {
            text.insert(text.begin() + cur_pos.y + 1,
                        text[cur_pos.y].substr(cur_pos.x));
            text[cur_pos.y] = text[cur_pos.y].substr(0, cur_pos.x);
            cur_pos.y++;
            cur_pos.x = 0;
          }
          break;
        }
        case 127: {
          if (mode == Insert) {
            if (cur_pos.x != 0 || cur_pos.y != 0) {
              if (cur_pos.x == 0 && cur_pos.y > 0) {
                size_t tmp = text[cur_pos.y - 1].length();
                text[cur_pos.y - 1] += text[cur_pos.y];
                text.erase(text.cbegin() + cur_pos.y);
                cur_pos.y--;
                cur_pos.x = tmp;
              } else {
                text[cur_pos.y] = text[cur_pos.y].substr(0, cur_pos.x - 1) +
                                  text[cur_pos.y].substr(cur_pos.x);
                cur_pos.x--;
              }
            }
            break;
          }
        }
        case ':': {
          if (mode == Normal) {
            // 命令系统
            int tmp;
            std::string cmd = ":";
            show_info(screen, cmd);
            screen->show();
            while ((tmp = getch()) != '\n') {
              if (tmp == 127 && cmd.length() > 1)
                cmd.pop_back();
              else
                cmd += tmp;
              show_info(screen, cmd);
              screen->show();
            }
            if (cmd == ":q") {
              screen->clear();
              screen->show();
              return;
            } else if (cmd == ":version") {
              show_info(screen, "Lightpad 0.0.1 by FurryR");
              screen->show();
              flag = false;
              break;
            }
            show_info(screen, "E: Unknown lightpad command");
            screen->show();
            flag = false;
            break;
          }
        }
        case 'i': {
          if (mode == Normal) {
            mode = Insert;
            break;
          }
        }
        case 'w': {
          // 上键
          if (mode == Normal) {
            process_arrow(&cur_pos, &x_before, text, 'A');
            break;
          }
        }
        case 's': {
          // 下键
          if (mode == Normal) {
            process_arrow(&cur_pos, &x_before, text, 'B');
            break;
          }
        }
        case 'a': {
          // 左键
          if (mode == Normal) {
            process_arrow(&cur_pos, &x_before, text, 'D');
            break;
          }
        }
        case 'd': {
          // 右键
          if (mode == Normal) {
            process_arrow(&cur_pos, &x_before, text, 'C');
            break;
          }
        }
        default: {
          if (mode == Insert) {
            if (cmd == '\t') {
              text[cur_pos.y] = text[cur_pos.y].substr(0, cur_pos.x) +
                                std::string(TAB_SIZE, ' ') +
                                text[cur_pos.y].substr(cur_pos.x);
              cur_pos.x += TAB_SIZE;
            } else {
              text[cur_pos.y].insert(text[cur_pos.y].begin() + cur_pos.x, cmd);
              cur_pos.x++;
            }
          }
        }
      }
      if (flag) break;
    }
  }
}
int main() {
  // std::cout.sync_with_stdio(false);
  Screen screen = Screen(getsize());
  main_ui(&screen);
}