#ifndef _COMMAND_H
#define _COMMAND_H
#include <functional>
#include <map>

#include "./ui.h"
// argument, UI, Window list, index = success(true) | failed(false)
typedef std::function<bool(const std::string&, UI*, std::vector<TextArea>*,
                           size_t*)>
    Command;
// argument, UI, Window list | return = success(true) | failed(false)
// typedef std::function<bool(const std::string&, UI*, std::vector<TextArea>*)>
// StandardCommand;
typedef class Parser {
  std::map<std::string, Command> cmd;
  Command default_command;

 public:
  Parser() {}
  bool execute(const std::string& command, UI* ui,
               std::vector<TextArea>* window_list, size_t* index) const {
    size_t idx = command.find_first_of(' ');
    std::string name, arg;
    if (idx == std::string::npos) {
      name = command;
      arg = "";
    } else {
      name = command.substr(0, idx);
      arg = command.substr(name.length() + 1);
    }
    if (cmd.find(name) != cmd.cend() && cmd.at(name)) {
      return cmd.at(name)(arg, ui, window_list, index);
    }
    if (default_command)
      return default_command(command, ui, window_list, index);
    return true;
  }
  void set_default(const Command& fn) { default_command = fn; }
  void set(const std::string& name, const Command& fn) { cmd[name] = fn; }
} Parser;
#endif