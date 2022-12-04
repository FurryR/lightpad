#ifndef _UI_H
#define _UI_H
#include <algorithm>
#include <fstream>
#include <functional>
#include <iterator>

#include "./screen.h"
#include "./utils.h"
#define TAB_SIZE 2
typedef class UI {
  Screen* screen;

 public:
  typedef struct Modestr {
    std::string prefix;
    std::string mode;
    Modestr(const std::string& mode, const std::string& prefix)
        : prefix(prefix), mode(mode) {}
  } Modestr;
  // 用于帮助渲染滚屏。当光标达到上边界或下边界时被使用。
  typedef enum class YRenderTextFlag {
    None = 0,           // 一般的情况。
    UpperBoundary = 1,  // 上边界。
    LowerBoundary = 2,  // 下边界。
  } YRenderTextFlag;
  // 用于帮助渲染滚屏。当光标达到左边界或右边界时被使用。
  typedef enum class XRenderTextFlag {
    None = 0,           // 一般的情况。
    LeftBoundary = 1,   // 上边界。
    RightBoundary = 2,  // 下边界。
  } XRenderTextFlag;
  std::pair<XRenderTextFlag, YRenderTextFlag> show_text(
      const std::vector<std::vector<Character>>& text, const Coord& cursor,
      size_t xoffset = 0, size_t yoffset = 0) const {
    size_t vec_index = yoffset, str_index = 0;
    size_t y = 0, x = 0;
    XRenderTextFlag flag = XRenderTextFlag::None;
    YRenderTextFlag flag2 = YRenderTextFlag::None;
    const Coord& sz = screen->size();
    for (; y < sz.y - 2 && vec_index < text.size(); y++, vec_index++) {
      str_index = xoffset;
      for (x = 0; x < sz.x && str_index < text[vec_index].size();
           x++, str_index++) {
        if (vec_index == cursor.y && str_index == cursor.x) {
          if (x == 0) {
            flag = XRenderTextFlag::LeftBoundary;
          } else if (x == screen->size().x - 1) {
            flag = XRenderTextFlag::RightBoundary;
          }
          if (y == 0) {
            flag2 = YRenderTextFlag::UpperBoundary;
          } else if (y == screen->size().y - 3) {
            flag2 = YRenderTextFlag::LowerBoundary;
          }
          if (text[vec_index][str_index].prefix == "") {
            screen->set(Coord(x, y),
                        Character(text[vec_index][str_index].content,
                                  "\x1b[38;5;247m\x1b[47m"));
          } else
            screen->set(
                Coord(x, y),
                Character(text[vec_index][str_index].content,
                          text[vec_index][str_index].prefix + "\x1b[47m"));
        } else {
          screen->set(Coord(x, y), text[vec_index][str_index]);
        }
      }
      if (vec_index == cursor.y && str_index == cursor.x) {
        if (x == 0) {
          flag = XRenderTextFlag::LeftBoundary;
        } else if (x == screen->size().x - 1) {
          flag = XRenderTextFlag::RightBoundary;
        }
        if (y == 0) {
          flag2 = YRenderTextFlag::UpperBoundary;
        } else if (y == screen->size().y - 3) {
          flag2 = YRenderTextFlag::LowerBoundary;
        }
        screen->set(Coord(x, y), Character(0, "\x1b[47m"));
      }
    }
    if (vec_index == cursor.y) {
      if (x == 0) {
        flag = XRenderTextFlag::LeftBoundary;
      } else if (x == screen->size().x - 1) {
        flag = XRenderTextFlag::RightBoundary;
      }
      if (y == 0) {
        flag2 = YRenderTextFlag::UpperBoundary;
      } else if (y == screen->size().y - 3) {
        flag2 = YRenderTextFlag::LowerBoundary;
      }
      screen->set(Coord(0, y), Character(0, "\x1b[47m"));
    }
    return {flag, flag2};
  }
  void show_bar(const Modestr& mode, const std::string& hint,
                const std::string& back_str) const {
    // 基层
    for (size_t i = 0; i < screen->size().x; i++) {
      screen->set(Coord(i, screen->size().y - 2), Character(0, "\x1b[44m"));
    }
    size_t counter = 0;
    // 左侧
    screen->set(Coord(counter++, screen->size().y - 2),
                Character(0, mode.prefix));
    for (size_t i = 0; i < mode.mode.length(); i++, counter++) {
      screen->set(Coord(counter, screen->size().y - 2),
                  Character(mode.mode[i], mode.prefix));
    }
    screen->set(Coord(counter++, screen->size().y - 2),
                Character(0, mode.prefix));
    // 中间
    screen->set(Coord(counter++, screen->size().y - 2),
                Character(0, "\x1b[44m"));
    for (size_t i = 0; i < hint.length(); counter++, i++) {
      screen->set(Coord(counter, screen->size().y - 2),
                  Character(hint[i], "\x1b[97;44m"));
    }
    // 右侧
    counter = screen->size().x - 1;
    screen->set(Coord(counter, screen->size().y - 2), Character(0, "\x1b[43m"));
    for (size_t i = back_str.length(); counter--, i--;) {
      screen->set(Coord(counter, screen->size().y - 2),
                  Character(back_str[i], "\x1b[30;43m"));
    }
    screen->set(Coord(counter, screen->size().y - 2), Character(0, "\x1b[43m"));
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
typedef enum Mode { Normal = 0, Insert = 1, Select = 2 } Mode;
UI::Modestr mode2str(Mode mode) {
  switch (mode) {
    case Normal:
      return UI::Modestr("NORMAL", "\x1b[42m\x1b[30m");
    case Insert:
      return UI::Modestr("INSERT", "\x1b[43m\x1b[30m");
    case Select:
      return UI::Modestr("SELECT", "\x1b[47m\x1b[30m");
  }
  return UI::Modestr("UNKNOWN", "");
}
typedef class TextArea {
  typedef class ScreenPos {
    // @since v0.1.1 用于保存当前光标位置。
    Coord cur_pos;

   public:
    // @since v0.1.1 用于控制文本列偏移。
    size_t xoffset;
    // @since v0.1.1 用于控制文本行偏移。
    size_t yoffset;
    // @since v0.1.1 用于保存文本渲染状态。
    UI::XRenderTextFlag xrender_flag;
    // @since v0.1.1 用于保存文本渲染状态。
    UI::YRenderTextFlag yrender_flag;
    void add_y(size_t max_y) noexcept {
      if (cur_pos.y < max_y) {
        if (yrender_flag == UI::YRenderTextFlag::LowerBoundary) yoffset++;
        cur_pos.y++;
      }
    }
    void sub_y() noexcept {
      if (cur_pos.y > 0) {
        if (yrender_flag == UI::YRenderTextFlag::UpperBoundary && yoffset > 0)
          yoffset--;
        cur_pos.y--;
      }
    }
    void set_y(size_t y, size_t screen_y) noexcept {
      cur_pos.y = y;
      if (yoffset < y && (y - yoffset) >= screen_y) {
        // 更小
        yoffset = y <= screen_y ? 0 : (y - screen_y);
        yrender_flag = UI::YRenderTextFlag::None;
      } else if (yoffset > y) {
        // 更大
        yoffset = y <= screen_y ? 0 : (y - screen_y);
        yrender_flag = UI::YRenderTextFlag::None;
      }
    }
    void add_x(size_t max_x) noexcept {
      if (cur_pos.x < max_x) {
        if (xrender_flag == UI::XRenderTextFlag::RightBoundary) xoffset++;
        cur_pos.x++;
      }
    }
    void sub_x() noexcept {
      if (cur_pos.x > 0) {
        if (xrender_flag == UI::XRenderTextFlag::LeftBoundary && xoffset > 0)
          xoffset--;
        cur_pos.x--;
      }
    }
    void set_x(size_t x, size_t screen_x) noexcept {
      cur_pos.x = x;
      if (xoffset < x && (x - xoffset) >= screen_x) {
        // 更小
        xoffset = x <= screen_x ? 0 : (x - screen_x);
        xrender_flag = UI::XRenderTextFlag::None;
      } else if (xoffset >= x) {
        // 更大
        xoffset = x <= screen_x ? 0 : (x - screen_x);
        xrender_flag = UI::XRenderTextFlag::None;
      }
    }
    const Coord& pos() const noexcept { return cur_pos; }
    ScreenPos() {}
    ScreenPos(const Coord& cur_pos, size_t xoffset, size_t yoffset,
              UI::XRenderTextFlag xrender_flag,
              UI::YRenderTextFlag yrender_flag)
        : cur_pos(cur_pos),
          xoffset(xoffset),
          yoffset(yoffset),
          xrender_flag(xrender_flag),
          yrender_flag(yrender_flag) {}
  } ScreenPos;
  std::vector<std::string> text;
  std::vector<std::vector<Character>> cache;
  std::string filename;
  std::function<std::vector<std::vector<Character>>(
      const std::vector<std::string>&)>
      renderer;
  ScreenPos pos;
  std::pair<Coord, Coord> select_pos;
  size_t select_yoffset;
  size_t x_before;
  Mode mode;
  bool dirty;
  bool file_changed;
  bool readonly;

 public:
  TextArea() {}
  TextArea(const std::function<std::vector<std::vector<Character>>(
               const std::vector<std::string>&)>& renderer,
           const std::vector<std::string>& text, bool readonly)
      : text(text),
        renderer(renderer),
        pos(Coord(0, 0), 0, 0, UI::XRenderTextFlag::None,
            UI::YRenderTextFlag::None),
        select_pos({Coord(0, 0), Coord(0, 0)}),
        select_yoffset(0),
        x_before(0),
        mode(Normal),
        dirty(true),
        file_changed(false),
        readonly(readonly) {}
  TextArea(const std::function<std::vector<std::vector<Character>>(
               const std::vector<std::string>&)>& renderer,
           const std::string& filename, bool readonly)
      : filename(filename),
        renderer(renderer),
        pos(Coord(0, 0), 0, 0, UI::XRenderTextFlag::None,
            UI::YRenderTextFlag::None),
        select_pos({Coord(0, 0), Coord(0, 0)}),
        select_yoffset(0),
        x_before(0),
        mode(Normal),
        dirty(true),
        file_changed(false),
        readonly(readonly) {
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
  std::vector<std::string>& get_text() noexcept { return text; }
  void set_pos(const UI& ui, const Coord& new_pos) noexcept {
    pos.set_x(new_pos.x, ui.size().x - 1);
    pos.set_y(new_pos.y, ui.size().y - 3);
  }
  Coord& start_range() noexcept { return select_pos.first; }
  Coord& end_range() noexcept { return select_pos.second; }
  const Coord& start_range() const noexcept { return select_pos.first; }
  const Coord& end_range() const noexcept { return select_pos.second; }
  Mode get_mode() const noexcept { return mode; }
  void select(const Coord& _start_range, const Coord& _end_range) noexcept {
    mode = Select;
    start_range() = _start_range;
    end_range() = _end_range;
  }
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
  std::vector<std::vector<Character>> select_render(
      const std::vector<std::vector<Character>>& c) const {
    std::vector<std::vector<Character>> ret = c;
    for (size_t y = start_range().y; y < end_range().y + 1; y++) {
      for (size_t x = (y == start_range().y ? start_range().x : 0);
           y == end_range().y
               ? (x < std::min<size_t>({ret[y].size(), end_range().x + 1}))
               : (x < ret[y].size());
           x++) {
        if (ret[y][x].prefix == "")
          ret[y][x].prefix = "\x1b[38;5;247m\x1b[48;5;250m";
        else
          ret[y][x].prefix += "\x1b[48;5;250m";
      }
    }
    return ret;
  }
  void render(UI* ui, const std::string& info) {
    ui->clear();
    if (dirty) {
      cache = renderer(text);
      dirty = false;
    }
    // pos.yrender_flag = ui->show_text(
    //     mode == Select ? select_render(cache) : cache, pos.pos(),
    //     (size_t)std::max<int>({0, int(pos.pos().x - ui->size().x + 1)}),
    //     pos.yoffset);
    std::pair<UI::XRenderTextFlag, UI::YRenderTextFlag> temp =
        ui->show_text(mode == Select ? select_render(cache) : cache, pos.pos(),
                      pos.xoffset, pos.yoffset);
    pos.xrender_flag = temp.first, pos.yrender_flag = temp.second;
    if (filename == "") {
      ui->show_bar(mode2str(mode),
                   std::string("(unnamed)") + (file_changed ? "[+]" : ""),
                   std::string("ln: ") + std::to_string(pos.pos().y + 1) + "/" +
                       std::to_string(text.size()) +
                       " col: " + std::to_string(pos.pos().x + 1));
    } else {
      ui->show_bar(mode2str(mode),
                   filename.substr(filename.find_last_of('/') + 1) +
                       (file_changed ? "[+]" : ""),
                   std::string("ln: ") + std::to_string(pos.pos().y + 1) + "/" +
                       std::to_string(text.size()) +
                       " col: " + std::to_string(pos.pos().x + 1));
    }
    ui->show_info(info);
    ui->update();
  }
  void _process_insert(const UI& screen, char op) {
    bool flag = false;
    if (readonly) return;
    file_changed = true;
    if (mode == Select) {
      dirty = true;
      flag = true;
      if (start_range().y == end_range().y) {
        text[start_range().y] =
            text[start_range().y].substr(0, start_range().x) +
            text[start_range().y].substr(
                (end_range().x == text[start_range().y].size())
                    ? end_range().x
                    : (end_range().x + 1));
      } else {
        text.erase(text.cbegin() + start_range().y,
                   text.cbegin() + end_range().y - 1);
        text[start_range().y] =
            text[start_range().y].substr(0, start_range().x) +
            text[start_range().y + 1].substr(
                (end_range().x == text[start_range().y + 1].size())
                    ? end_range().x
                    : (end_range().x + 1));
        text.erase(text.cbegin() + start_range().y + 1);
      }
      pos.set_x(start_range().x, screen.size().x - 1);
      pos.set_y(start_range().y, screen.size().y - 3);
      start_range() = end_range() = Coord(0, 0);
      select_yoffset = 0;
      mode = Insert;
    }
    switch (op) {
      case '\n': {
        // 换行
        text.insert(text.begin() + pos.pos().y + 1,
                    text[pos.pos().y].substr(pos.pos().x));
        text[pos.pos().y] = text[pos.pos().y].substr(0, pos.pos().x);
        pos.add_y(text.size() - 1);
        pos.set_x(0, screen.size().x - 1);
        dirty = true;
        break;
      }
      case 127: {
        // 退格
        if (!flag) {
          if (pos.pos().x != 0 || pos.pos().y != 0) {
            if (pos.pos().x == 0 && pos.pos().y > 0) {
              size_t tmp = text[pos.pos().y - 1].length();
              text[pos.pos().y - 1] += text[pos.pos().y];
              text.erase(text.cbegin() + pos.pos().y);
              pos.sub_y();
              pos.set_x(tmp, screen.size().x - 1);
            } else {
              text[pos.pos().y] = text[pos.pos().y].substr(0, pos.pos().x - 1) +
                                  text[pos.pos().y].substr(pos.pos().x);
              pos.sub_x();
            }
            dirty = true;
          }
        }
        break;
      }
      default: {
        if (op == '\t') {
          text[pos.pos().y] = text[pos.pos().y].substr(0, pos.pos().x) +
                              std::string(TAB_SIZE, ' ') +
                              text[pos.pos().y].substr(pos.pos().x);
          // pos.pos().x += TAB_SIZE;
          pos.set_x(pos.pos().x + 2, screen.size().x - 1);
        } else {
          text[pos.pos().y].insert(text[pos.pos().y].begin() + pos.pos().x, op);
          pos.add_x(text[pos.pos().y].length());
        }
        dirty = true;
      }
    }
  }
  void process_key(const UI& screen, char op) {
    switch (op) {
      case '\x1b': {
        // Esc, 方向键
        if (kbhit() && getch() == '[') {
          process_arrow(screen, getch());
        } else {
          if (mode == Select) {
            start_range() = end_range() = Coord(0, 0);
            select_yoffset = 0;
          }
          // 切换为普通模式
          mode = Normal;
        }
        break;
      }
      case 'I':
      case 'i': {
        // 插入模式
        if (mode == Normal && (!readonly)) {
          mode = Insert;
        } else if (mode != Select)
          _process_insert(screen, op);
        break;
      }
      case 'W':
      case 'w': {
        // 上键
        if (mode == Normal || mode == Select) {
          process_arrow(screen, 'A');
        } else
          _process_insert(screen, op);
        break;
      }
      case 'S':
      case 's': {
        // 下键
        if (mode == Normal || mode == Select) {
          process_arrow(screen, 'B');
        } else
          _process_insert(screen, op);
        break;
      }
      case 'A':
      case 'a': {
        // 左键
        if (mode == Normal || mode == Select) {
          process_arrow(screen, 'D');
        } else
          _process_insert(screen, op);
        break;
      }
      case 'D':
      case 'd': {
        // 右键
        if (mode == Normal || mode == Select) {
          process_arrow(screen, 'C');
        } else
          _process_insert(screen, op);
        break;
      }
      case 'V':
      case 'v': {
        // 右键
        if (mode == Normal && (!readonly)) {
          mode = Select;
          start_range() = end_range() = pos.pos();
          select_yoffset = pos.yoffset;
        } else if (mode != Select)
          _process_insert(screen, op);
        break;
      }
      case 127: {
        if (mode == Insert || mode == Select) {
          _process_insert(screen, op);
        }
        break;
      }
      default: {
        if (mode == Insert) {
          _process_insert(screen, op);
        }
        break;
      }
    }
  }
  void process_arrow(const UI& screen, char op) {
    switch (op) {
      // up
      case 'A': {
        if (pos.pos().y > 0) {
          pos.sub_y();
          if (x_before >= text[pos.pos().y].length()) {
            pos.set_x(text[pos.pos().y].length(), screen.size().x - 1);
          } else {
            pos.set_x(x_before, screen.size().x - 1);
          }
          if (mode == Select) {
            if (start_range().y > pos.pos().y ||
                (start_range().y == pos.pos().y &&
                 start_range().x > pos.pos().x) ||
                (start_range().x == end_range().x &&
                 start_range().y == end_range().y)) {
              select_yoffset = pos.yoffset;
              start_range() = pos.pos();
            } else
              end_range() = pos.pos();
          }
        }
        break;
      }
      // down
      case 'B': {
        if (pos.pos().y < text.size() - 1) {
          pos.add_y(text.size() - 1);
          if (x_before >= text[pos.pos().y].length()) {
            pos.set_x(text[pos.pos().y].length(), screen.size().x - 1);
          } else {
            pos.set_x(x_before, screen.size().x - 1);
          }
          if (mode == Select) {
            if (end_range().y > pos.pos().y ||
                (end_range().y == pos.pos().y && end_range().x > pos.pos().x)) {
              select_yoffset = pos.yoffset;
              start_range() = pos.pos();
            } else
              end_range() = pos.pos();
          }
        }
        break;
      }
      // right
      case 'C': {
        if (pos.pos().x < text[pos.pos().y].length())
          pos.add_x(text[pos.pos().y].length());
        else if (pos.pos().y < text.size() - 1) {
          pos.add_y(text.size() - 1);
          pos.set_x(0, screen.size().x - 1);
        }
        x_before = pos.pos().x;
        if (mode == Select) {
          if (end_range().y > pos.pos().y ||
              (end_range().y == pos.pos().y && end_range().x > pos.pos().x)) {
            select_yoffset = pos.yoffset;
            start_range() = pos.pos();
          } else
            end_range() = pos.pos();
        }
        break;
      }
      // left
      case 'D': {
        if (pos.pos().x > 0)
          pos.sub_x();
        else if (pos.pos().y > 0) {
          pos.sub_y();
          pos.set_x(text[pos.pos().y].size(), screen.size().x - 1);
        }
        x_before = pos.pos().x;
        if (mode == Select) {
          if (start_range().y > pos.pos().y ||
              (start_range().y == pos.pos().y &&
               start_range().x > pos.pos().x) ||
              (start_range().x == end_range().x &&
               start_range().y == end_range().y)) {
            select_yoffset = pos.yoffset;
            start_range() = pos.pos();
          } else
            end_range() = pos.pos();
        }
        break;
      }
    }
  }
} TextArea;

#endif
