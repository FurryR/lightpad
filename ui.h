#ifndef _UI_H
#define _UI_H
#include <fstream>
#include <functional>
#include <iterator>

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
  // 用于帮助渲染滚屏。当光标达到上边界或下边界时被使用。
  typedef enum class RenderTextFlag {
    None = 0,          // 一般的情况。
    UpperBoundary = 1,    // 上边界。
    LowerBoundary = 2  // 下边界。
  } RenderTextFlag;
  Screen* screen;
  RenderTextFlag show_text(const std::vector<std::vector<Character>>& text,
                           const Coord& cursor, size_t xoffset = 0,
                           size_t* yoffset = 0) const {
    size_t vec_index = *yoffset, str_index = 0;
    size_t y = 0, x = 0;
    RenderTextFlag flag = RenderTextFlag::None;
    const Coord& sz = screen->size();
    for (; y < sz.y - 2; y++) {
      str_index = xoffset;
      if (vec_index < text.size()) {
        for (x = 0; x < sz.x; x++) {
          if (str_index < text[vec_index].size()) {
            if (vec_index == cursor.y && str_index == cursor.x) {
              if (y == 0) {
                flag = RenderTextFlag::UpperBoundary;
              } else if (y == screen->size().y - 3) {
                flag = RenderTextFlag::LowerBoundary;
              }
              screen->set(
                  Coord(x, y),
                  Character(text[vec_index][str_index].content,
                            text[vec_index][str_index].prefix + "\x1b[47m"));
            } else {
              screen->set(Coord(x, y), text[vec_index][str_index]);
            }
            str_index++;
          } else
            break;
        }
        if (vec_index == cursor.y && str_index == cursor.x) {
          if (y == 0) {
            flag = RenderTextFlag::UpperBoundary;
          } else if (y == screen->size().y - 3) {
            flag = RenderTextFlag::LowerBoundary;
          }
          screen->set(Coord(x, y), Character(0, "\x1b[47m"));
        }
      } else
        break;
      vec_index++;
    }
    if (vec_index == cursor.y) {
      if (y == 0) {
        flag = RenderTextFlag::UpperBoundary;
      } else if (y == screen->size().y - 3) {
        flag = RenderTextFlag::LowerBoundary;
      }
      screen->set(Coord(0, y), Character(0, "\x1b[47m"));
    }
    return flag;
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
                    Character(hint[idx], "\x1b[97;44m"));
        idx++;
      } else {
        screen->set(Coord(i, screen->size().y - 2), Character(0, "\x1b[44m"));
      }
    }
    // 右侧
    screen->set(Coord(screen->size().x - 1, screen->size().y - 2),
                Character(0, "\x1b[43m"));
    for (size_t i = 0; i < back_str.length(); i++) {
      screen->set(Coord(screen->size().x - (back_str.length() - i) - 1,
                        screen->size().y - 2),
                  Character(back_str[i], "\x1b[30;43m"));
    }
    screen->set(
        Coord(screen->size().x - back_str.length() - 2, screen->size().y - 2),
        Character(0, "\x1b[43m"));
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
      return UI::Modestr("NORMAL", "\x1b[42m\x1b[30m");
    case Insert:
      return UI::Modestr("INSERT", "\x1b[43m\x1b[30m");
  }
  return UI::Modestr("UNKNOWN", "");
}
typedef class TextArea {
  Coord cur_pos;
  size_t x_before;
  std::vector<std::string> text;
  std::vector<std::vector<Character>> cache;
  std::string filename;
  bool dirty;
  bool file_changed;
  bool readonly;
  // @since v0.1.0 用于控制文本行偏移。
  size_t yoffset;
  // @since v0.1.0 用于保存文本渲染状态。
  UI::RenderTextFlag render_flag;
  std::function<std::vector<std::vector<Character>>(
      const std::vector<std::string>&)>
      renderer;
  Mode mode;

 public:
  TextArea() {}
  TextArea(const std::function<std::vector<std::vector<Character>>(
               const std::vector<std::string>&)>& renderer,
           const std::vector<std::string>& text, bool readonly)
      : cur_pos(Coord(0, 0)),
        x_before(0),
        yoffset(0),
        text(text),
        dirty(true),
        file_changed(false),
        readonly(readonly),
        renderer(renderer),
        mode(Normal),
        render_flag(UI::RenderTextFlag::None) {}
  TextArea(const std::function<std::vector<std::vector<Character>>(
               const std::vector<std::string>&)>& renderer,
           const std::string& filename, bool readonly)
      : cur_pos(Coord(0, 0)),
        x_before(0),
        yoffset(0),
        filename(filename),
        dirty(true),
        file_changed(false),
        readonly(readonly),
        renderer(renderer),
        mode(Normal),
        render_flag(UI::RenderTextFlag::None) {
    std::ifstream fs = std::ifstream(filename);
    if (!fs) {
      text.push_back("");
      return;
    }
    std::string raw = std::string(std::istreambuf_iterator<char>(fs),
                                  std::istreambuf_iterator<char>());
    fs.close();
    std::string tmp = "";
    for (size_t i = 0; i < raw.length(); i++) {
      if (raw[i] == '\n') {
        text.push_back(tmp);
        tmp = "";
      } else
        tmp += raw[i];
    }
    if (tmp != "") text.push_back(tmp);
    if (text.size() == 0) text.push_back("");
  }

 public:
  Mode get_mode() const noexcept { return mode; }
  bool get_readonly() const noexcept { return readonly; }
  bool write(const std::string& dst) {
    std::ofstream fs = std::ofstream(dst);
    if (!fs) {
      return false;
    }
    filename = dst;
    for (size_t i = 0; i < text.size(); i++) {
      if (i + 1 != text.size()) {
        fs << text[i] << "\n";
      } else {
        fs << text[i];
      }
    }
    file_changed = false;
    return true;
  }
  const std::string& get_filename() const noexcept { return filename; }
  bool is_changed() const noexcept { return file_changed; }
  void switch_renderer(const std::function<std::vector<std::vector<Character>>(
                           const std::vector<std::string>&)>& new_renderer) {
    renderer = new_renderer;
    dirty = true;
  }
  void render(UI* ui, const std::string& info) {
    ui->clear();
    if (dirty) {
      cache = renderer(text);
      dirty = false;
    }
    render_flag = ui->show_text(
        cache, cur_pos,
        (size_t)std::max<int>({0, int(cur_pos.x - ui->size().x + 1)}),
        &yoffset);
    if (filename == "") {
      ui->show_bar(mode2str(mode),
                   std::string("[unnamed]") + (file_changed ? "[+]" : ""),
                   std::string("ln: ") + std::to_string(cur_pos.y + 1) + "/" +
                       std::to_string(text.size()) +
                       " col: " + std::to_string(cur_pos.x + 1));
    } else {
      ui->show_bar(mode2str(mode),
                   filename.substr(filename.find_last_of('/') + 1) +
                       (file_changed ? "[+]" : ""),
                   std::string("ln: ") + std::to_string(cur_pos.y + 1) + "/" +
                       std::to_string(text.size()) +
                       " col: " + std::to_string(cur_pos.x + 1));
    }
    ui->show_info(info);
    ui->update();
  }
  void _process_insert(char op) {
    if (readonly) return;
    file_changed = true;
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
      case '\x1b': {
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
        if (mode == Normal && (!readonly)) {
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
          if (render_flag == UI::RenderTextFlag::LowerBoundary && yoffset > 0) yoffset--;
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
          if (render_flag == UI::RenderTextFlag::UpperBoundary) yoffset++;
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

#endif