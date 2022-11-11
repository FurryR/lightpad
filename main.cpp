#include <algorithm>
#include <functional>

#include "./render/tomorrow-night-bright-cpp.h"
#include "./render/tomorrow-night-bright-js.h"
#include "./screen.h"
#include "./utils.h"
#define TAB_SIZE 2
typedef struct UI {
  typedef struct Modestr {
    std::string prefix;
    std::string mode;
    Modestr(const std::string& mode, const std::string& prefix)
        : prefix(prefix), mode(mode) {}
  } Modestr;
  Screen* screen;
  void show_text(const std::vector<std::vector<Character>>& text,
                 const Coord& cursor, size_t xoffset = 0,
                 size_t yoffset = 0) const {
    size_t vec_index = yoffset, str_index = 0;
    size_t y = 0, x = 0;
    const Coord& sz = screen->size();
    for (; y < sz.y - 2; y++) {
      str_index = xoffset;
      if (vec_index < text.size()) {
        for (x = 0; x < sz.x; x++) {
          if (str_index < text[vec_index].size()) {
            if (vec_index == cursor.y && str_index == cursor.x) {
              screen->set(
                  Coord(x, y),
                  Character(text[vec_index][str_index].content,
                            text[vec_index][str_index].prefix + "\e[47m"));
            } else {
              screen->set(Coord(x, y), text[vec_index][str_index]);
            }
            str_index++;
          } else
            break;
        }
        if (vec_index == cursor.y && str_index == cursor.x) {
          screen->set(Coord(x, y), Character(0, "\e[47m"));
        }
      } else
        break;
      vec_index++;
    }
    if (vec_index == cursor.y) {
      screen->set(Coord(0, y), Character(0, "\e[47m"));
    }
  }
  void show_bar(const Modestr& mode, const std::string& hint,
                const std::string& back_str) const {
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
                    Character(hint[idx], "\e[97;44m"));
        idx++;
      } else {
        screen->set(Coord(i, screen->size().y - 2), Character(0, "\e[44m"));
      }
    }
    // 右侧
    screen->set(Coord(screen->size().x - 1, screen->size().y - 2),
                Character(0, "\e[43m"));
    for (size_t i = 0; i < back_str.length(); i++) {
      screen->set(Coord(screen->size().x - (back_str.length() - i) - 1,
                        screen->size().y - 2),
                  Character(back_str[i], "\e[30;43m"));
    }
    screen->set(
        Coord(screen->size().x - back_str.length() - 2, screen->size().y - 2),
        Character(0, "\e[43m"));
  }
  void show_info(const std::string& info) const {
    for (size_t x = 0; x < screen->size().x; x++) {
      screen->set(Coord(x, screen->size().y - 1), Character(0));
    }
    for (size_t i = 0; i < info.length(); i++) {
      screen->set(Coord(i, screen->size().y - 1), Character(info[i]));
    }
  }
  void update() const { screen->show(); }
  void clear() const { screen->clear(); }
  const Coord& size() const noexcept { return screen->size(); }
  UI(Screen* screen) : screen(screen) {}
} UI;

namespace PlainText {
// 默认的 render。不会给代码上色。
std::vector<std::vector<Character>> render(
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
};  // namespace PlainText
typedef enum Mode { Normal = 0, Insert = 1 } Mode;
UI::Modestr mode2str(Mode mode) {
  switch (mode) {
    case Normal:
      return UI::Modestr("NORMAL", "\e[42m\e[30m");
    case Insert:
      return UI::Modestr("INSERT", "\e[43m\e[30m");
  }
  return UI::Modestr("UNKNOWN", "");
}
typedef struct TextArea {
  Coord cur_pos;
  size_t x_before;
  std::vector<std::string> text;
  std::vector<std::vector<Character>> cache;
  bool dirty;
  std::function<std::vector<std::vector<Character>>(
      const std::vector<std::string>&)>
      renderer;
  Mode mode;
  TextArea(const std::function<std::vector<std::vector<Character>>(
               const std::vector<std::string>&)>& renderer,
           const std::vector<std::string>& text)
      : cur_pos(Coord(0, 0)),
        x_before(0),
        text(text),
        dirty(true),
        renderer(renderer),
        mode(Normal) {}
  void render(UI* ui) {
    ui->clear();
    if (dirty) {
      cache = renderer(text);
      dirty = false;
    }
    ui->show_text(
        cache, cur_pos,
        (size_t)std::max<int>({0, int(cur_pos.x - ui->size().x)}),
        (size_t)std::max<int>({0, int(cur_pos.y - ui->size().y + 3)}));
    ui->show_bar(mode2str(mode), "[unnamed]",
                 std::string("ln: ") + std::to_string(cur_pos.y + 1) + "/" +
                     std::to_string(text.size()) +
                     " col: " + std::to_string(cur_pos.x + 1));
    ui->update();
  }
  void _process_insert(char op) {
    switch (op) {
      case '\n': {
        // 换行
        text.insert(text.begin() + cur_pos.y + 1,
                    text[cur_pos.y].substr(cur_pos.x));
        text[cur_pos.y] = text[cur_pos.y].substr(0, cur_pos.x);
        cur_pos.y++;
        cur_pos.x = 0;
        dirty = true;
        break;
      }
      case 127: {
        // 退格
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
          dirty = true;
        }
        break;
      }
      default: {
        if (op == '\t') {
          text[cur_pos.y] = text[cur_pos.y].substr(0, cur_pos.x) +
                            std::string(TAB_SIZE, ' ') +
                            text[cur_pos.y].substr(cur_pos.x);
          cur_pos.x += TAB_SIZE;
        } else {
          text[cur_pos.y].insert(text[cur_pos.y].begin() + cur_pos.x, op);
          cur_pos.x++;
        }
        dirty = true;
      }
    }
  }
  void process_key(char op) {
    switch (op) {
      case '\e': {
        // Esc, 方向键
        if (kbhit() == '[') {
          // 方向键
          getch();
          process_arrow(getch());
        } else {
          // 切换为普通模式
          mode = Normal;
        }
        break;
      }
      case 'i': {
        // 插入模式
        if (mode == Normal) {
          mode = Insert;
        } else
          _process_insert(op);
        break;
      }
      case 'w': {
        // 上键
        if (mode == Normal) {
          process_arrow('A');
        } else
          _process_insert(op);
        break;
      }
      case 's': {
        // 下键
        if (mode == Normal) {
          process_arrow('B');
        } else
          _process_insert(op);
        break;
      }
      case 'a': {
        // 左键
        if (mode == Normal) {
          process_arrow('D');
        } else
          _process_insert(op);
        break;
      }
      case 'd': {
        // 右键
        if (mode == Normal) {
          process_arrow('C');
        } else
          _process_insert(op);
        break;
      }
      default: {
        if (mode == Insert) {
          _process_insert(op);
        }
        break;
      }
    }
  }
  void process_arrow(char op) {
    switch (op) {
      // up
      case 'A': {
        if (cur_pos.y > 0) {
          cur_pos.y--;
          if (x_before >= text[cur_pos.y].length()) {
            cur_pos.x = text[cur_pos.y].length();
          } else {
            cur_pos.x = x_before;
          }
        }
        break;
      }
      // down
      case 'B': {
        if (cur_pos.y < text.size() - 1) {
          cur_pos.y++;
          if (x_before >= text[cur_pos.y].length()) {
            cur_pos.x = text[cur_pos.y].length();
          } else {
            cur_pos.x = x_before;
          }
        }
        break;
      }
      // left
      case 'D': {
        if (cur_pos.x > 0)
          cur_pos.x--;
        else if (cur_pos.y > 0) {
          cur_pos.y--;
          cur_pos.x = text[cur_pos.y].size();
        }
        x_before = cur_pos.x;
        break;
      }
      // right
      case 'C': {
        if (cur_pos.x < text[cur_pos.y].length())
          cur_pos.x++;
        else if (cur_pos.y < text.size() - 1) {
          cur_pos.y++;
          cur_pos.x = 0;
        }
        x_before = cur_pos.x;
        break;
      }
    }
  }
} TextArea;

void main_ui(Screen* screen) {
  std::vector<std::string> text;
  text.push_back("#include <iostream>");
  text.push_back("// An example c++ program rendered by lightpad-cpprender.");
  text.push_back("int main() {");
  text.push_back("  std::cout << \"Hello World!\\n\" << std::endl;");
  text.push_back("  return 0;");
  text.push_back("}");
  UI ui = UI(screen);
  TextArea textarea = TextArea(TomorrowNightBrightCpp::render, text);
  int cmd = 0;
  while (1) {
    textarea.render(&ui);
    while (1) {
      bool flag = true;
      cmd = getch();
      switch (cmd) {
        case ':': {
          if (textarea.mode == Normal) {
            // 命令系统
            bool execute = true;
            int tmp;
            std::string cmd = ":";
            ui.show_info(cmd);
            ui.update();
            while ((tmp = getch()) != '\n') {
              if (tmp == 127) {
                if (cmd.length() > 1) cmd.pop_back();
              } else if (tmp == '\e') {
                execute = false;
                break;
              } else
                cmd += tmp;
              ui.show_info(cmd);
              ui.update();
            }
            if (execute == false) {
              break;
            }
            if (cmd == ":q") {
              ui.clear();
              ui.update();
              return;
            } else if (cmd == ":version") {
              ui.show_info("Lightpad 0.0.2 by FurryR");
              ui.update();
              flag = false;
              break;
            } else if (cmd == ":cpp_render") {
              // use tomorrow-night-bright-cpp
              textarea.renderer = TomorrowNightBrightCpp::render;
              textarea.dirty =
                  true;  // force rendering after switching renderer
              ui.show_info("Switch to TomorrowNightBrightCpp::render");
              ui.update();
              flag = false;
              break;
            } else if (cmd == ":plaintext_render") {
              // use plaintext
              textarea.renderer = PlainText::render;
              textarea.dirty =
                  true;  // force rendering after switching renderer
              ui.show_info("Switch to PlainText::render");
              ui.update();
              flag = false;
              break;
            } else if (cmd == ":js_render") {
              // use tomorrow-night-bright-js
              textarea.renderer = TomorrowNightBrightJs::render;
              textarea.dirty = true;
              ui.show_info("Switch to TomorrowNightBrightJs::render");
              ui.update();
              flag = false;
              break;
            }
            ui.show_info("E: Unknown lightpad command");
            ui.update();
            flag = false;
          } else
            textarea.process_key(cmd);
          break;
        }
        default: {
          textarea.process_key(cmd);
          break;
        }
      }
      if (flag) break;
    }
  }
}
int main() {
  termios tm;
  tcgetattr(STDIN_FILENO, &tm);
  tm.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &tm);
  // std::cout.sync_with_stdio(false);
  Screen screen = Screen(getsize());
  main_ui(&screen);
}