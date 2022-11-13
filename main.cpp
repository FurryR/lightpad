#include <algorithm>
#include <fstream>

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
  ui->show_bar(mode2str(Normal), "[unnamed]", "ln: 0/0 col: 0");
  ui->show_info("[0/0]");
  ui->update();
}
std::vector<std::string> help_message = {
    "Lightpad v0.1.0 Help",
    "",
    "Keys: ",
    "WASD(NORMAL mode)/Arrow - Move cursor",
    "i(NORMAL mode) - Switch to INSERT mode",
    "Esc(INSERT mode) - Switch to NORMAL mode",
    ":(NORMAL mode) - Command",
    "",
    "Commands: ",
    ":lang [cpp | js | plain] - Switch renderer for the code",
    ":w (filename) - Write to file",
    ":q - Quit/Close current tab",
    ":q! - Force quit/close current tab (not recommended)",
    ":new (filename) - Open a new tab (open filename if filename is specified)",
    ":help - Display this help"};
Parser get_command() {
  Parser temp;
  temp.set(
      ":version",
      [](const std::string&, UI* ui, std::vector<TextArea>*, size_t*) -> bool {
        ui->show_info("Lightpad v0.1.0 by FurryR");
        ui->update();
        return false;
      });
  temp.set(":lang",
           [](const std::string& arg, UI* ui,
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
           [](const std::string&, UI* ui, std::vector<TextArea>* window_list,
              size_t* index) -> bool {
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
           [](const std::string&, UI* ui, std::vector<TextArea>* window_list,
              size_t* index) -> bool {
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
  temp.set(":help",
           [](const std::string&, UI*, std::vector<TextArea>* window_list,
              size_t* index) -> bool {
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
           [](const std::string& args, UI*, std::vector<TextArea>* window_list,
              size_t* index) -> bool {
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
      [](const std::string& args, UI* ui, std::vector<TextArea>* window_list,
         size_t* index) -> bool {
        if (window_list->size() == 0) {
          ui->show_info("E: No tabs available");
          ui->update();
          return false;
        } else {
          if ((*window_list)[*index].get_readonly()) {
            ui->show_info("E: File is readonly");
            ui->update();
            return false;
          }
          if (args == "") {
            if ((*window_list)[*index].get_filename() == "") {
              ui->show_info("E: No file name");
              ui->update();
              return false;
            }
            std::string filename = (*window_list)[*index].get_filename();
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
                if ((*window_list)[i].get_filename() == args) {
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
  temp.set_default(
      [](const std::string&, UI* ui, std::vector<TextArea>*, size_t*) -> bool {
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
void main_ui(Screen* screen, const std::vector<std::string>& args) {
  Parser parser = get_command();
  UI ui = UI(screen);
  std::vector<TextArea> window;
  if (args.size() != 0) {
    window.push_back(open_file(args[0]));
  }
  size_t window_index = 0;
  // TextArea textarea = TextArea(TomorrowNightBrightCpp::render, text);
  int cmd = 0;
  while (1) {
    if (window.size() > 0) {
      window[window_index].render(&ui,
                                  generate_window_list(window, window_index));
    } else {
      render_default_ui(&ui);
    }
    while (1) {
      bool flag = true;
      cmd = getch();
      switch (cmd) {
        case ':': {
          if (window.size() == 0 || window[window_index].get_mode() == Normal) {
            // 命令系统
            bool execute = true;
            int tmp;
            std::string cmd = ":";
            ui.show_info(cmd);
            ui.update();
            while ((tmp = getch()) != '\n') {
              if (tmp == 127) {
                if (cmd.length() > 1) cmd.pop_back();
              } else if (tmp == '\x1b') {
                execute = false;
                break;
              } else
                cmd += tmp;
              ui.show_info(cmd);
              ui.update();
            }
            if (!execute) break;
            flag = parser.execute(cmd, &ui, &window, &window_index);
          } else if (window.size() != 0)
            window[window_index].process_key(cmd);
          break;
        }
        case 'z': {
          // 上一个窗口
          if (window.size() > 0 || window[window_index].get_mode() == Normal) {
            if (window_index > 0) window_index--;
          } else if (window.size() != 0)
            window[window_index].process_key(cmd);
          break;
        }
        case 'x': {
          // 下一个窗口
          if (window.size() > 0 || window[window_index].get_mode() == Normal) {
            if (window_index < window.size() - 1) window_index++;
          } else if (window.size() != 0)
            window[window_index].process_key(cmd);
          break;
        }
        default: {
          if (window.size() != 0) window[window_index].process_key(cmd);
          break;
        }
      }
      if (flag) break;
    }
  }
}
int main(int argc, char** argv) {
  termios tm;
  tcgetattr(STDIN_FILENO, &tm);
  tm.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &tm);
  std::vector<std::string> s(argc - 1);
  for (int i = 1; i < argc; i++) {
    s[i - 1] = argv[i];
  }
  // std::cout.sync_with_stdio(false);
  Screen screen = Screen(getsize());
  main_ui(&screen, s);
}