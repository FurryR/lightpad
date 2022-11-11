#ifndef _TOMORROW_NIGHT_BRIGHT_CPP_H
#define _TOMORROW_NIGHT_BRIGHT_CPP_H
#include <algorithm>
#include <array>

#include "../screen.h"
namespace TomorrowNightBrightCpp {
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
  PreProcessor = 5,
  String = 6,
  LineComment = 7,
  Literal = 8
} TokenType;
bool isnum(const std::string& p) {
  try {
    for (size_t i = 0; i < p.length(); i++) {
      if ((p[i] >= L'0' && p[i] <= L'9') || p[i] == L'.' || p[i] == L'e')
        ;
      else
        return false;
    }
    if (p.find_first_of('.') != std::wstring::npos ||
        p.find_first_of('e') != std::wstring::npos)
      return std::stod(p), true;
    else
      return std::stoi(p, 0, 0), true;
  } catch (...) {
    return false;
  }
}
std::string _render_color(TokenType type) {
  switch (type) {
    case Keyword:
      return "\e[35m";  // 关键字
    case Operator:
      return "\e[36m";  // 算符
    case Identifier:
      return "\e[37m";  // 普通标识符
    case Number:
      return "\e[33m";  // 数字
    case PreProcessor:
      return "\e[35m";  //预处理器
    case String:
      return "\e[32m";  // 字符串
    case LineComment:
      return "\e[38;5;242m";  // 行注释
    case Literal:
      return "\e[34m";  // 常量
    default:
      return "\e[37m";
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
void _transfer(wchar_t i, size_t& z, size_t& a, size_t& j) {
  if (i == L'\\')
    z = !z;
  else if (i == L'\"' && !z) {
    if (a == 0 || a == 1) a = (a == 0 ? 1 : 0);
  } else if (i == L'\'' && !z) {
    if (a == 0 || a == 2) a = (a == 0 ? 2 : 0);
  } else
    z = false;
  if ((i == L'(' || i == L'{' || i == L'[') && a == 0)
    j++;
  else if ((i == L')' || i == L'}' || i == L']') && a == 0)
    j--;
}
std::array<std::string, 60> keyword = {"bool",
                                       "short",
                                       "int",
                                       "long",
                                       "float",
                                       "double",
                                       "inline",
                                       "auto",
                                       "restrict",
                                       "explicit",
                                       "typeid",
                                       "asm",
                                       "alignas",
                                       "alignof",
                                       "class",
                                       "enum",
                                       "struct",
                                       "operator",
                                       "union",
                                       "signed",
                                       "unsigned",
                                       "typedef",
                                       "const",
                                       "volatile",
                                       "static",
                                       "extern",
                                       "noexcept",
                                       "constexpr",
                                       "void",
                                       "typename",
                                       "char",
                                       "break",
                                       "continue",
                                       "for",
                                       "if",
                                       "else",
                                       "while",
                                       "do",
                                       "switch",
                                       "case",
                                       "default",
                                       "try",
                                       "catch",
                                       "throw",
                                       "template",
                                       "using",
                                       "return",
                                       "namespace",
                                       "public",
                                       "protected",
                                       "private",
                                       "new",
                                       "static_assert",
                                       "delete",
                                       "sizeof",
                                       "decltype",
                                       "const_cast",
                                       "static_cast",
                                       "dynamic_cast",
                                       "reinterpret_cast"};
std::array<std::string, 4> literal = {"true", "false", "NULL", "nullptr"};
ColorText _get_colortext(const std::string& tmp) {
  if (std::find(keyword.cbegin(), keyword.cend(), tmp) != keyword.cend()) {
    return ColorText(tmp, _render_color(Keyword));
  } else if (isnum(tmp)) {
    return ColorText(tmp, _render_color(Number));
  } else if (std::find(literal.cbegin(), literal.cend(), tmp) !=
             literal.cend()) {
    return ColorText(tmp, _render_color(Literal));
  } else if (isIdentifier(tmp)) {
    return ColorText(tmp, _render_color(Identifier));
  } else {
    return ColorText(tmp, _render_color(None));
  }
}
std::array<char, 13> separator = {' ', ';', ',',  '{',  '}', '[', ']',
                                  '(', ')', '\"', '\'', ':', '.'};

std::array<char, 13> op = {'+', '-', '*', '/', '>', '<', '=',
                           '!', '&', '|', '%', '^', '~'};
std::vector<ColorText> _render_one(const std::string& text) {
  std::string tmp;
  std::vector<ColorText> ret;
  for (size_t i = 0, a = 0, j = 0, z = 0; i < text.length(); i++) {
    _transfer(text[i], z, a, j);
    if (a == 1 || a == 2) {
      tmp += text[i];
      i++;
      while (i < text.length() && a != 0) {
        _transfer(text[i], z, a, j);
        tmp += text[i];
        i++;
      }
      ret.push_back(ColorText(tmp, _render_color(String)));
      tmp = "";
    }
    if (tmp == "//" && a == 0) {
      ret.push_back(
          ColorText(tmp + text.substr(i), _render_color(LineComment)));
      tmp = "";
      break;
    } else if (tmp == "#" && a == 0) {
      ret.push_back(
          ColorText(tmp + text.substr(i), _render_color(PreProcessor)));
      tmp = "";
      break;
    } else if (std::find(separator.cbegin(), separator.cend(), text[i]) !=
               separator.cend()) {
      ret.push_back(_get_colortext(tmp));
      tmp = "";
      if (text[i] != '\"' && text[i] != '\'')
        ret.push_back(ColorText(std::string(1, text[i]), ""));
    } else if (std::find(op.cbegin(), op.cend(), text[i]) != op.cend() &&
               (text[i] != '/' ||
                !((i + 1 < text.length() && text[i + 1] == '/') ||
                  (i >= 1 && text[i - 1] == '/')))) {
      ret.push_back(_get_colortext(tmp));
      ret.push_back(
          ColorText(std::string(1, text[i]), _render_color(Operator)));
      tmp = "";
    } else
      tmp += text[i];
  }
  if (tmp != "") ret.push_back(ColorText(tmp, _render_color(None)));
  return ret;
}
std::vector<std::vector<ColorText>> _render(
    const std::vector<std::string>& text) {
  std::vector<std::vector<ColorText>> ret(text.size());
  for (size_t i = 0; i < text.size(); i++) {
    ret[i] = _render_one(text[i]);
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
}  // namespace TomorrowNightBrightCpp
#endif
