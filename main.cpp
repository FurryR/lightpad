#include <signal.h>

#include <algorithm>
#include <fstream>

#include "./awacorn/generator.h"
#include "./command.h"
#include "./render/tomorrow-night-bright-cpp.h"
#include "./render/tomorrow-night-bright-js.h"
#include "./ui.h"

TextArea open_file(const std::string& filename) {
  if (filename.size() > 3 &&
      (filename.substr(filename.length() - 4) == ".cpp" ||
       filename.substr(filename.length() - 4) == ".hpp" ||
       filename.substr(filename.length() - 4) == ".cxx"))
    return TextArea(TomorrowNightBrightCpp::render, filename, false);
  else if (filename.size() > 2 &&
           filename.substr(filename.length() - 3) == ".js")
    return TextArea(TomorrowNightBrightJs::render, filename, false);
  else if (filename.size() > 1 &&
           (filename.substr(filename.length() - 2) == ".h" ||
            filename.substr(filename.length() - 2) == ".c"))
    return TextArea(TomorrowNightBrightCpp::render, filename, false);
  else
    return TextArea(PlainText::render, filename, false);
}
void render_default_ui(UI* ui) {
  ui->clear();
  ui->show_bar(mode2str(Normal), "(unnamed)", "ln: 0/0 col: 0");
  ui->show_info("[0/0]");
  ui->update();
}
const std::vector<std::string> help_message = {
    "--- Lightpad v0.2.0 Help ---",
    "",
    "Keys: ",
    "  WSAD(NORMAL mode)/Arrow - Move cursor",
    "  i(NORMAL mode) - Switch to INSERT mode",
    "  Esc(INSERT/SELECT mode) - Switch to NORMAL mode",
    "  :(NORMAL mode) - Command",
    "  z(NORMAL mode) - Previous tab",
    "  x(NORMAL mode) - Next tab",
    "  v(NORMAL mode) - Enter SELECT mode (use WSAD/Arrow to select text)",
    "  Backspace(SELECT mode) - Remove selected text",
    "",
    "Commands: ",
    "  :lang [cpp | js | plain] - Switch renderer for the code",
    "  :w (filename) - Write to file",
    "  :q - Quit/Close current tab",
    "  :q! - Force quit/close current tab (not recommended)",
    "  :new (filename) - Open a new tab (open filename if filename is "
    "specified)",
    "  :find [content] - Searches the file for the first string [content].",
    "  :help - Display this help",
    "  :version - Display the version of Lightpad",
    "",
    "For example, use \":new\" to open a new tab and use \":lang cpp\" to "
    "switch the renderer.",
    "",
    "Enjoy Lightpad."};
Parser get_command() {
  Parser temp;
  temp.set(":version",
           [](const std::string&, Awacorn::EventLoop*, UI* ui,
              std::vector<TextArea>*, size_t*) -> bool {
             ui->show_info("Lightpad v0.2.0 by FurryR");
             ui->update();
             return false;
           });
  temp.set(":lang",
           [](const std::string& arg, Awacorn::EventLoop*, UI* ui,
              std::vector<TextArea>* window_list, size_t* index) -> bool {
             if (window_list->size() != 0) {
               if (arg == "cpp") {
                 (*window_list)[*index].switch_renderer(
                     TomorrowNightBrightCpp::render);
                 return true;
               } else if (arg == "js") {
                 (*window_list)[*index].switch_renderer(
                     TomorrowNightBrightJs::render);
                 return true;
               } else if (arg == "plain") {
                 (*window_list)[*index].switch_renderer(PlainText::render);
                 return true;
               } else {
                 ui->show_info("E: Unknown language");
                 ui->update();
                 return false;
               }
             } else {
               ui->show_info("E: No tabs available");
               ui->update();
               return false;
             }
           });
  temp.set(":q",
           [](const std::string&, Awacorn::EventLoop*, UI* ui,
              std::vector<TextArea>* window_list, size_t* index) -> bool {
             if (window_list->size() != 0) {
               if ((*window_list)[*index].is_changed()) {
                 ui->show_info("E: No write since last change (:q! overrides)");
                 ui->update();
                 return false;
               } else {
                 window_list->erase(window_list->begin() + (*index));
                 if ((*index) != 0) (*index)--;
                 return true;
               }
             } else {
               ui->clear();
               ui->update();
               exit(0);
               return true;
             }
           });
  temp.set(":q!",
           [](const std::string&, Awacorn::EventLoop*, UI* ui,
              std::vector<TextArea>* window_list, size_t* index) -> bool {
             if (window_list->size() != 0) {
               window_list->erase(window_list->begin() + (*index));
               if ((*index) != 0) (*index)--;
               return true;
             } else {
               ui->clear();
               ui->update();
               exit(0);
               return true;
             }
           });
  temp.set(":find",
           [](const std::string& arg, Awacorn::EventLoop*, UI* ui,
              std::vector<TextArea>* window_list, size_t* index) -> bool {
             if (window_list->size() != 0) {
               if ((*window_list)[*index].get_readonly()) {
                 ui->show_info("E: File is read-only");
                 ui->update();
                 return false;
               }
               for (size_t y = 0; y < (*window_list)[*index].get_text().size();
                    y++) {
                 size_t pos = (*window_list)[*index].get_text()[y].find(arg);
                 if (pos != std::string::npos) {
                   (*window_list)[*index].set_pos(
                       *ui, Coord(pos + arg.length() - 1, y));
                   (*window_list)[*index].select(
                       Coord(pos, y), Coord(pos + arg.length() - 1, y));
                   return true;
                 }
               }
               ui->show_info("E: Nothing matched");
               ui->update();
               return false;
             } else {
               ui->show_info("E: No tabs available");
               ui->update();
               return false;
             }
           });
  temp.set(":help",
           [](const std::string&, Awacorn::EventLoop*, UI*,
              std::vector<TextArea>* window_list, size_t* index) -> bool {
             if (window_list->size() == 0) {
               window_list->push_back(
                   TextArea(PlainText::render, help_message, true));
               return true;
             } else {
               window_list->insert(
                   window_list->begin() + (*index) + 1,
                   TextArea(PlainText::render, help_message, true));
               (*index)++;
               return true;
             }
           });
  temp.set(":new",
           [](const std::string& args, Awacorn::EventLoop*, UI*,
              std::vector<TextArea>* window_list, size_t* index) -> bool {
             if (window_list->size() == 0) {
               if (args == "")
                 window_list->push_back(TextArea(
                     PlainText::render, std::vector<std::string>({""}), false));
               else
                 window_list->push_back(open_file(args));
               return true;
             } else {
               if (args == "") {
                 window_list->insert(
                     window_list->begin() + (*index) + 1,
                     TextArea(PlainText::render, std::vector<std::string>({""}),
                              false));
                 (*index)++;
                 return true;
               }
               for (size_t i = 0; i < window_list->size(); i++) {
                 std::string a = (*window_list)[i].get_filename();
                 if (a != "" && a == args) {
                   (*index) = i;
                   return true;
                 }
               }
               window_list->insert(window_list->begin() + (*index) + 1,
                                   open_file(args));
               (*index)++;
               return true;
             }
           });
  temp.set(
      ":w",
      [](const std::string& args, Awacorn::EventLoop*, UI* ui,
         std::vector<TextArea>* window_list, size_t* index) -> bool {
        if (window_list->size() == 0) {
          ui->show_info("E: No tabs available");
          ui->update();
          return false;
        } else {
          if ((*window_list)[*index].get_readonly()) {
            ui->show_info("E: File is read-only");
            ui->update();
            return false;
          }
          if (args == "") {
            if ((*window_list)[*index].get_filename() == "") {
              ui->show_info("E: No file name");
              ui->update();
              return false;
            }
            const std::string& filename = (*window_list)[*index].get_filename();
            if (!(*window_list)[*index].write(filename)) {
              ui->show_info("E: Write file failed");
              ui->update();
            } else {
              ui->show_info("\"" +
                            filename.substr(filename.find_last_of('/') + 1) +
                            "\" written");
              ui->update();
            }
            return false;
          } else {
            if (!(*window_list)[*index].write(args)) {
              ui->show_info("E: Write file failed");
              ui->update();
            } else {
              for (size_t i = 0; i < window_list->size(); i++) {
                // 如果找到相同的Window，则覆盖，然后删除这个。
                if (i != (*index) && (*window_list)[i].get_filename() == args) {
                  (*window_list)[i] = (*window_list)[*index];
                  size_t backup = *index;
                  (*index) = i;
                  window_list->erase(window_list->begin() + backup);
                  break;
                }
              }
              ui->show_info("\"" + args.substr(args.find_last_of('/') + 1) +
                            "\" written");
              ui->update();
            }
            return false;
          }
          window_list->insert(window_list->begin() + (*index) + 1,
                              open_file(args));
          (*index)++;
          return true;
        }
      });
  temp.set_default([](const std::string&, Awacorn::EventLoop*, UI* ui,
                      std::vector<TextArea>*, size_t*) -> bool {
    ui->show_info("E: Unknown lightpad command");
    ui->update();
    return false;
  });
  return temp;
}
std::string _generate_one_window(const TextArea& area, size_t index) {
  std::string filename = area.get_filename();
  if (filename == "") {
    return "[" + std::to_string(index + 1) + "] " + "(unnamed)";
  }
  return "[" + std::to_string(index + 1) + "] " +
         filename.substr(filename.find_last_of('/') + 1);
}
std::string generate_window_list(const std::vector<TextArea>& window,
                                 size_t index) {
  std::string ret = "[" + std::to_string(index + 1) + "/" +
                    std::to_string(window.size()) + "] ";
  // 如果可以的话，将index向前挪2格。
  if (index > 0) index--;
  if (index > 0) index--;
  for (size_t i = index; i < window.size(); i++) {
    ret += _generate_one_window(window[i], i);
    if (i + 1 < window.size()) ret += " ";
  }
  return ret;
}
Awacorn::AsyncFn<Promise::Promise<void>> main_ui(
    Screen* screen, const std::vector<std::string>& args) {
  return [screen, args](Awacorn::EventLoop* ev) {
    return Generator::AsyncGenerator<void>(
               [ev, screen,
                args](Generator::AsyncGenerator<void>::Context* ctx) {
                 Parser parser = get_command();
                 UI ui(screen);
                 std::vector<TextArea> window{};
                 size_t window_index = 0;
                 int cmd = 0;
                 bool flag = true;
                 for (size_t i = 0; i < args.size(); i++) {
                   window.push_back(open_file(args[i]));
                 }
                 ev->create(
                     [&window, &ui, &window_index, &flag](
                         Awacorn::EventLoop*,
                         const Awacorn::Interval*) -> void {
                       if (flag) {
                         if (window.size() > 0) {
                           window[window_index].render(
                               &ui, generate_window_list(window, window_index));
                           ui.update();
                         } else {
                           render_default_ui(&ui);
                         }
                       }
                     },
                     std::chrono::milliseconds(7));
                 while (1) {
                   cmd = ctx->await(ev->run(async_getch()));
                   flag = true;
                   switch (cmd) {
                     case ':': {
                       if (window.size() == 0 ||
                           window[window_index].get_mode() == Normal) {
                         // 命令系统
                         flag = false;
                         std::string tmp(":");
                         int key = 0;
                         ui.show_info(tmp);
                         ui.update();
                         while (1) {
                           key = ctx->await(ev->run(async_getch()));
                           if (key == '\n') {
                             flag = parser.execute(tmp, ev, &ui, &window,
                                                   &window_index);
                             break;
                           } else if (key == '\x1b') {
                             flag = true;
                             break;
                           } else if (key == 127) {
                             if (tmp.length() > 1) tmp.pop_back();
                           } else {
                             tmp += key;
                           }
                           ui.show_info(tmp);
                           ui.update();
                         }
                       } else if (window.size() != 0)
                         window[window_index].process_key(ui, cmd);
                       break;
                     }
                     case 'Z':
                     case 'z': {
                       // 上一个窗口
                       if (window.size() > 0 &&
                           window[window_index].get_mode() == Normal) {
                         if (window_index > 0) window_index--;
                       } else if (window.size() != 0)
                         window[window_index].process_key(ui, cmd);
                       break;
                     }
                     case 'X':
                     case 'x': {
                       // 下一个窗口
                       if (window.size() > 0 &&
                           window[window_index].get_mode() == Normal) {
                         if (window_index < window.size() - 1) window_index++;
                       } else if (window.size() != 0)
                         window[window_index].process_key(ui, cmd);
                       break;
                     }
                     default: {
                       if (window.size() != 0)
                         window[window_index].process_key(ui, cmd);
                       break;
                     }
                   }
                 }
               })
        .next();
  };
}
termios tm;
void exit_fn(int) {
  std::cout << std::endl;
  tcgetattr(STDIN_FILENO, &tm);
  tm.c_lflag |= ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &tm);
  exit(0);
}
int main(int argc, char** argv) {
  // termios tm;
  tcgetattr(STDIN_FILENO, &tm);
  tm.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &tm);
  signal(SIGINT, exit_fn);
  atexit([]() -> void { exit_fn(0); });
  Awacorn::EventLoop ev;
  std::vector<std::string> s(argc - 1);
  for (int i = 1; i < argc; i++) {
    s[i - 1] = argv[i];
  }
  Screen screen = Screen(getsize());
  ev.run(main_ui(&screen, s));
  ev.start();
}
