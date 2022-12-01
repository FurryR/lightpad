#ifndef _TOMORROW_NIGHT_BRIGHT_JS_H
#define _TOMORROW_NIGHT_BRIGHT_JS_H
#include <algorithm>
#include <array>

#include "../screen.h"
namespace TomorrowNightBrightJs {
typedef struct ColorText {
  std::string prefix;
  std::string content;
  std::vector<Character> output() const {
    std::vector<Character> ret = std::vector<Character>(content.length());
    for (size_t i = 0; i < content.length(); i++)
      ret[i] = Character(content[i], prefix);
    return ret;
  }
  ColorText() {}
  ColorText(const std::string& content, const std::string& prefix)
      : prefix(prefix), content(content) {}
} ColorText;
typedef enum TokenType {
  None = 0,
  Keyword = 1,
  Operator = 2,
  Identifier = 3,
  Number = 4,
  String = 5,
  Comment = 6,
  Literal = 7
} TokenType;
bool isnum(const std::string& p) {
  if (p == "Infinity" || p == "NaN") return true;
  for (size_t i = 0; i < p.length(); i++) {
    if ((p[i] >= '0' && p[i] <= '9') || (p[i] >= 'A' && p[i] <= 'F') ||
        (p[i] >= 'a' && p[i] <= 'f') || p[i] == 'x' || p[i] == 'X' ||
        p[i] == '.' || p[i] == 'e')
      ;
    else
      return false;
  }
  return true;
}
std::string _render_color(TokenType type) {
  switch (type) {
    case Keyword:
      return "\x1b[35m";  // 关键字
    case Operator:
      return "\x1b[36m";  // 算符
    case Identifier:
      return "";  // 普通标识符
    case Number:
      return "\x1b[33m";  // 数字
    case String:
      return "\x1b[32m";  // 字符串
    case Comment:
      return "\x1b[38;5;242m";  // 注释
    case Literal:
      return "\x1b[34m";  // 常量
    default:
      return "";
  }
}
bool isIdentifier(const std::string& x) {
  if (x.length() == 0) return false;
  bool flag = false;
  for (size_t i = 0; i < x.length(); i++) {
    if (i == 1) flag = true;
    if (x[i] >= '0' && x[i] <= '9') {
      if (!flag) return false;
    } else if ((x[i] >= 'a' && x[i] <= 'z') || (x[i] >= 'A' && x[i] <= 'Z') ||
               x[i] == '_')
      continue;
    else
      return false;
  }
  return true;
}
void _transfer(wchar_t i, size_t& z, size_t& a) {
  if (i == L'\\')
    z = !z;
  else if (i == L'\"' && !z) {
    if (a == 0 || a == 1) a = (a == 0 ? 1 : 0);
  } else if (i == L'\'' && !z) {
    if (a == 0 || a == 2) a = (a == 0 ? 2 : 0);
  } else
    z = false;
}
constexpr std::array<const char*, 35> keyword = {
    "this",  "function", "class", "yield",  "async",    "await",      "new",
    "super", "delete",   "void",  "typeof", "in",       "instanceof", "import",
    "break", "continue", "if",    "else",   "switch",   "case",       "default",
    "throw", "try",      "catch", "var",    "let",      "const",      "return",
    "do",    "while",    "of",    "export", "debugger", "from", "as"};
constexpr std::array<const char*, 4> literal = {"true", "false", "undefined", "null"};
ColorText _get_colortext(const std::string& tmp) {
  if (std::find(keyword.cbegin(), keyword.cend(), tmp) != keyword.cend()) {
    return ColorText(tmp, _render_color(Keyword));
  } else if (std::find(literal.cbegin(), literal.cend(), tmp) !=
             literal.cend()) {
    return ColorText(tmp, _render_color(Literal));
  } else if (isIdentifier(tmp)) {
    return ColorText(tmp, _render_color(Identifier));
  } else if (isnum(tmp)) {
    return ColorText(tmp, _render_color(Number));
  } else {
    return ColorText(tmp, _render_color(None));
  }
}
std::array<char, 13> separator = {' ', ';', ',',  '{',  '}', '[', ']',
                                  '(', ')', '\"', '\'', ':', '.'};

std::array<char, 13> op = {'+', '-', '*', '/', '>', '<', '=',
                           '!', '&', '|', '%', '^', '~'};
std::vector<ColorText> _render_one(const std::string& text, bool* status) {
  std::string tmp;
  std::vector<ColorText> ret;
  for (size_t i = 0, a = 0, z = 0; i < text.length(); i++) {
    if (*status) {
      for (; i < text.length() &&
             (tmp.length() < 2 || tmp.substr(tmp.length() - 2, 2) != "*/");
           i++) {
        tmp += text[i];
      }
      if (i == text.length() &&
          (tmp.length() < 2 || tmp.substr(tmp.length() - 2, 2) != "*/"))
        break;
      *status = false;
      ret.push_back(ColorText(tmp, _render_color(Comment)));
      tmp = "";
    }
    _transfer(text[i], z, a);
    if (a == 1 || a == 2) {
      tmp += text[i];
      i++;
      for (; i < text.length() && a != 0; i++) {
        _transfer(text[i], z, a);
        tmp += text[i];
      }
      ret.push_back(ColorText(tmp, _render_color(String)));
      tmp = "";
    }
    if (tmp + text[i] == "//" && a == 0) {
      ret.push_back(ColorText(tmp + text.substr(i), _render_color(Comment)));
      tmp = "";
      break;
    } else if (tmp + text[i] == "/*" && a == 0) {
      *status = true;
      tmp += text[i];
      continue;
    } else if (std::find(separator.cbegin(), separator.cend(), text[i]) !=
               separator.cend()) {
      ret.push_back(_get_colortext(tmp));
      tmp = "";
      if (text[i] != '\"' && text[i] != '\'')
        ret.push_back(ColorText(std::string(1, text[i]), _render_color(None)));
    } else if (std::find(op.cbegin(), op.cend(), text[i]) != op.cend()) {
      if (text[i] != '/' && text[i] != '*') {
        ret.push_back(_get_colortext(tmp));
        ret.push_back(
            ColorText(std::string(1, text[i]), _render_color(Operator)));
        tmp = "";
      } else {
        if (text[i] == '/' && i + 1 < text.length() &&
            (text[i + 1] == '*' || text[i + 1] == '/')) {
          ret.push_back(_get_colortext(tmp));
          tmp = std::string(1, text[i]);
        } else if (text[i] == '*' && i > 0 && text[i - 1] == '/') {
          tmp += text[i];
        } else {
          ret.push_back(_get_colortext(tmp));
          ret.push_back(
              ColorText(std::string(1, text[i]), _render_color(Operator)));
          tmp = "";
        }
      }
    } else
      tmp += text[i];
  }
  if (tmp != "") {
    if (*status)
      ret.push_back(ColorText(tmp, _render_color(Comment)));
    else
      ret.push_back(_get_colortext(tmp));
  }
  return ret;
}
std::vector<std::vector<ColorText>> _render(
    const std::vector<std::string>& text) {
  std::vector<std::vector<ColorText>> ret(text.size());
  bool flag = false;
  for (size_t i = 0; i < text.size(); i++) {
    ret[i] = _render_one(text[i], &flag);
  }
  return ret;
}
std::vector<std::vector<Character>> render(
    const std::vector<std::string>& text) {
  std::vector<std::vector<ColorText>> tmp = _render(text);
  std::vector<std::vector<Character>> ret =
      std::vector<std::vector<Character>>(tmp.size());
  for (size_t i = 0; i < tmp.size(); i++) {
    for (size_t j = 0; j < tmp[i].size(); j++) {
      std::vector<Character>&& res = tmp[i][j].output();
      ret[i].insert(ret[i].cend(), res.begin(), res.end());
    }
  }
  return ret;
}
}  // namespace TomorrowNightBrightJs
#endif
