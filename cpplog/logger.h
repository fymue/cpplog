/*
 Copyright (c) 2023 Fynn Mürmanns

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#ifndef CPPLOG_LOGGER_H_
#define CPPLOG_LOGGER_H_

#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <ctime>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <utility>
#include <unordered_map>
#include "logdefinitions.h"
#include "logimpl.h"

namespace cpplog {

/*
* contains data of a to-be-formatted object from the format string;
* the format string can parse decimal numbers, floating point numbers,
* strings, objects and characters. Formatting can be specified in curly
* brackets with Python/printf-like syntax:
*   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
*   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
* supported format specifiers (have to be inside {...}):
*   - '>' : any character put before '>' will be used to left-pad
*     the to-be-printed type (if a max character size was also specified);
*     '>' should be the 2nd character in the specified format
*   - max character size: limit the number of characters the to-be-printed
*     output can have; this number should be either the first number
*     of the format or put after the '>' specifier (for left-padding)
*   - '.' : specify the number of decimal places after a floating-point
*     number after this character
*   - 'd' | 'f' | 's' | 'o' | 'c' | 'b' : specify that the to-be-printed
*     parameter is a decimal number, floating-point number, string, object,
*     character or boolean
*   - '<' : any character put after '<' will be used to right-pad
*     the to-be-printed type (if a max character size was also specified);
*     '>' should be the 2nd to last character in the specified format
*/
struct FormatStringObject {

  FormatStringObject();

  ~FormatStringObject() {}

  static inline void pad(std::ostream &stream, int n, char pad_chr);

  static bool is_number(char chr);

  static std::string get_type(FormatStringSpecifier type);

  // parse a (multi-digit) number from format string
  static uint16_t get_number(const char *fmt_str, int &fmt_str_idx);

  FormatStringSpecifier type;
  char left_pad_chr, right_pad_chr;
  uint16_t mx_len, mx_decimal_places;

  // start and end idx of the format specifier in the format string
  int start_idx, end_idx;
};

/*
 * The Logger class contains a reference to std::cerr, which
 * is the output stream for all logged messages;
 * you can specify the log level and log format of the
 * to-be-logged messages using the set_log_level and
 * set_log_format methods; this class contains methods that
 * support the logging of all primitive and most std library types;
 * if you wish to log your own custom types, please extend the
 * "LogImpl" class and add your own
 * "inline void log(std::ostream &stream, YourCustomType t, LogFormat fmt)"
 * definitions that specify how your custom types should be logged
 * and provide your extension of the LogImpl class as the template
 * class for the Logger class;
 * the Logger class will take ownership of the LogImpl object,
 * so be aware that it will be deleted whenever the Logger class
 * get deleted
 */
template<class LogImpl>
class Logger {

  public:

    Logger();

    Logger(const char *name, const LogImpl &log_impl);

    Logger(const char *name, LogLevel lvl, LogFormat fmt,
           const LogImpl &log_impl);

    ~Logger() {}

    // set log level
    void set_log_level(LogLevel lvl);

    // specify log format (see LogFormatOption enum for available options)
    void set_log_format(LogFormat fmt);

    /*
    * set the log implementation object;
    * the Logger class will copy the provided LogImpl object
    */
    void set_log_impl(const LogImpl &log_impl);

    template<typename T>
    void error(const T &t, LogFormat fmt);

    template<typename T>
    void error(const T &t);

    /*
    * print args accoring to format specified by fmt_str;
    * the format string can parse decimal numbers, floating point numbers,
    * strings, objects and characters. Formatting can be specified in curly
    * brackets with Python/printf-like syntax:
    *   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
    *   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
    */
    template<typename ...T>
    void error(const char *fmt_str, T&&... args);

    template<typename T, typename ...Tr>
    void error(const char *fmt_str, LogFormat fmt, T &&first, Tr&&... args);

    template<typename T>
    void warn(const T &t, LogFormat fmt);

    template<typename T>
    void warn(const T &t);

    /*
    * print args accoring to format specified by fmt_str;
    * the format string can parse decimal numbers, floating point numbers,
    * strings, objects and characters. Formatting can be specified in curly
    * brackets with Python/printf-like syntax:
    *   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
    *   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
    */
    template<typename ...T>
    void warn(const char *fmt_str, T&&... args);

    template<typename T, typename ...Tr>
    void warn(const char *fmt_str, LogFormat fmt, T &&first, Tr&&... args);

    template<typename T>
    void info(const T &t, LogFormat fmt);

    template<typename T>
    void info(const T &t);

    /*
    * print args accoring to format specified by fmt_str;
    * the format string can parse decimal numbers, floating point numbers,
    * strings, objects and characters. Formatting can be specified in curly
    * brackets with Python/printf-like syntax:
    *   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
    *   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
    */
    template<typename ...T>
    void info(const char *fmt_str, T&&... args);

    template<typename T, typename ...Tr>
    void info(const char *fmt_str, LogFormat fmt, T &&first, Tr&&... args);

  private:

    std::vector<FormatStringObject> _parse_format_string(const char *fmt_str);

    // handle max length and padding of to-be-printed format argument
    void _pad_fmt_arg(std::ostream &str, const std::string &arg,
                      const FormatStringObject &fmt_obj);

    template<typename T>
    void _log_fmt_arg(std::stringstream &fmt_arg, T &&_arg,
                      const char *log_fmt, int start_idx, int end_idx,
                      const FormatStringObject &fmt_obj);

    // terminate the variadic argument recursion and print the rest of
    // the text in the format string after the last format specifier
    void _log_format_string_args(std::stringstream &fmt_stream,
                                const std::vector<FormatStringObject> &fmt_objs,
                                const char *fmt_str,
                                int start_idx, int end_idx, int obj_idx);

    template<typename T, typename ...Tr>
    void _log_format_string_args(std::stringstream &fmt_stream,
                                const std::vector<FormatStringObject> &fmt_objs,
                                const char *fmt_str, int start_idx, int end_idx,
                                int obj_idx, T &&first, Tr &&...rest);

    LogLevel _log_lvl;
    std::ostream &_stream;
    std::string _name;
    LogFormat _log_format;
    LogImpl _log_impl;
    std::mutex _mutex;

    // default log formats for error, warning and info messages
    const LogFormat _default_err_fmt =
      LogFormatOption::HIGHLIGHT_RED |
      LogFormatOption::TIMESTAMP     |
      LogFormatOption::NEWLINE;

    const LogFormat _default_warn_fmt =
      LogFormatOption::HIGHLIGHT_YELLOW |
      LogFormatOption::TIMESTAMP        |
      LogFormatOption::NEWLINE;

    const LogFormat _default_info_fmt =
      LogFormatOption::HIGHLIGHT_GREEN |
      LogFormatOption::TIMESTAMP       |
      LogFormatOption::NEWLINE;
};

inline FormatStringObject::FormatStringObject() :
  type(NONE), left_pad_chr('\0'), right_pad_chr('\0'),
  mx_len(0), mx_decimal_places(0), start_idx(0), end_idx(0) {
}

inline void FormatStringObject::pad(std::ostream &stream, int n, char pad_chr) {
  if (n < 0) return;

  for (int i = 0; i < n; ++i) {
    stream << pad_chr;
  }
}

inline bool FormatStringObject::is_number(char chr) {
  return !(chr < '0' || chr > '9');
}

// parse a (multi-digit) number from format string
inline uint16_t FormatStringObject::get_number(const char *fmt_str,
                                               int &fmt_str_idx) {
  constexpr uint8_t MX_DIGITS = 3;
  char number[] = "0\0\0";
  int number_i = 0;

  while (number_i < MX_DIGITS &&
        FormatStringObject::is_number(fmt_str[fmt_str_idx])) {
    number[number_i++] = fmt_str[fmt_str_idx++];
  }

  return static_cast<uint16_t>(std::stoi(number));
}

template<class LogImpl>
inline Logger<LogImpl>::Logger() :
  _stream(std::cerr), _name("LOG") {
  set_log_level(LogLevel::STANDARD);
  set_log_format(LogLevel::STANDARD);
}

template<class LogImpl>
inline Logger<LogImpl>::Logger(const char *name, const LogImpl &log_impl) :
  _stream(std::cerr), _name(name) {
  set_log_level(LogLevel::STANDARD);
  set_log_format(LogLevel::STANDARD);
  set_log_impl(log_impl);
}

template<class LogImpl>
inline Logger<LogImpl>::Logger(const char *name, LogLevel lvl, LogFormat fmt,
                     const LogImpl &log_impl) :
  _log_lvl(lvl), _stream(std::cerr), _name(name), _log_format(fmt) {
  set_log_level(lvl);
  set_log_format(fmt);
  set_log_impl(log_impl);
}

// set log level
template<class LogImpl>
inline void Logger<LogImpl>::set_log_level(LogLevel lvl) {
  _log_lvl = lvl;
  _log_format = _log_lvl;
}

// specify log format (see LogFormatOption enum for available options)
template<class LogImpl>
inline void Logger<LogImpl>::set_log_format(LogFormat fmt) {
  _log_format = fmt;
}

/*
* set the log implementation object;
* the Logger class will take ownership of the LogImpl object,
* so be aware that it will be deleted whenever the Logger class
* get deleted
*/
template<class LogImpl>
inline void Logger<LogImpl>::set_log_impl(const LogImpl &log_impl) {
  _log_impl = log_impl;
  _log_impl.set_name(_name);
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::error(const T &t, LogFormat fmt) {
  std::lock_guard<std::mutex> lock(_mutex);
  _log_impl.log(_stream, t, fmt | _default_err_fmt);
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::error(const T &t) {
  error(t, _log_format);
}

/*
* print args accoring to format specified by fmt_str;
* the format string can parse decimal numbers, floating point numbers,
* strings, objects and characters. Formatting can be specified in curly
* brackets with Python/printf-like syntax:
*   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
*   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
*/
template<class LogImpl>
template<typename ...T>
inline void Logger<LogImpl>::error(const char *fmt_str, T&&... args) {
  error(fmt_str, _log_format, std::forward<T>(args)...);
}

template<class LogImpl>
template<typename T, typename ...Tr>
inline void Logger<LogImpl>::error(const char *fmt_str, LogFormat fmt,
                                   T &&first, Tr&&... args) {
  std::vector<FormatStringObject> objs = _parse_format_string(fmt_str);
  std::stringstream fmt_stream;
  _log_format_string_args(fmt_stream, objs, fmt_str, 0,
                          objs.front().start_idx, 0, std::move(first),
                          std::forward<Tr>(args)...);
  std::lock_guard<std::mutex> lock(_mutex);
  _log_impl.parse_fmt_opts(_stream, fmt_stream.rdbuf(),
                            fmt | _default_err_fmt);
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::warn(const T &t, LogFormat fmt) {
  warn(t, _log_format);
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::warn(const T &t) {
  std::lock_guard<std::mutex> lock(_mutex);
  _log_impl.log(_stream, t, _log_format | _default_warn_fmt);
}

/*
* print args accoring to format specified by fmt_str;
* the format string can parse decimal numbers, floating point numbers,
* strings, objects and characters. Formatting can be specified in curly
* brackets with Python/printf-like syntax:
*   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
*   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
*/
template<class LogImpl>
template<typename ...T>
inline void Logger<LogImpl>::warn(const char *fmt_str, T&&... args) {
  warn(fmt_str, _log_format, std::forward<T>(args)...);
}

template<class LogImpl>
template<typename T, typename ...Tr>
inline void Logger<LogImpl>::warn(const char *fmt_str, LogFormat fmt, T &&first,
                                  Tr&&... args) {
  std::vector<FormatStringObject> objs = _parse_format_string(fmt_str);
  std::stringstream fmt_stream;
  _log_format_string_args(fmt_stream, objs, fmt_str, 0,
                          objs.front().start_idx, 0, std::move(first),
                          std::forward<Tr>(args)...);
  std::lock_guard<std::mutex> lock(_mutex);
  _log_impl.parse_fmt_opts(_stream, fmt_stream.rdbuf(),
                            fmt | _default_warn_fmt);
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::info(const T &t, LogFormat fmt) {
  std::lock_guard<std::mutex> lock(_mutex);
  _log_impl.log(_stream, t, fmt | _default_info_fmt);
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::info(const T &t) {
  info(t, _log_format);
}

/*
* print args accoring to format specified by fmt_str;
* the format string can parse decimal numbers, floating point numbers,
* strings, objects and characters. Formatting can be specified in curly
* brackets with Python/printf-like syntax:
*   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
*   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
*/
template<class LogImpl>
template<typename ...T>
inline void Logger<LogImpl>::info(const char *fmt_str, T&&... args) {
  info(fmt_str, _log_format, std::forward<T>(args)...);
}

template<class LogImpl>
template<typename T, typename ...Tr>
inline void Logger<LogImpl>::info(const char *fmt_str, LogFormat fmt, T &&first,
                                  Tr&&... args) {
  std::vector<FormatStringObject> objs = _parse_format_string(fmt_str);
  std::stringstream fmt_stream;
  _log_format_string_args(fmt_stream, objs, fmt_str, 0,
                          objs.front().start_idx, 0, std::move(first),
                          std::forward<Tr>(args)...);
  std::lock_guard<std::mutex> lock(_mutex);
  _log_impl.parse_fmt_opts(_stream, fmt_stream.rdbuf(),
                            fmt | _default_info_fmt);
}

template<class LogImpl>
std::vector<FormatStringObject> Logger<LogImpl>::_parse_format_string(
  const char *fmt_str) {
  std::vector<FormatStringObject> fmt_objs;

  size_t str_len = std::strlen(fmt_str);

  for (int i = 0; i < str_len;) {
    if (fmt_str[i] == FormatStringSpecifier::OPEN) {
      FormatStringObject obj;
      obj.start_idx = i;
      ++i;

      // store left pad character (if specified, comes before '>')
      if (fmt_str[i + 1] == FormatStringSpecifier::PAD_LEFT) {
        obj.left_pad_chr = fmt_str[i];
        i += 2;
      }

      // store max number of characters of formatted argument
      obj.mx_len = FormatStringObject::get_number(fmt_str, i);

      // store max number of decimal places after floating point number
      if (fmt_str[i] == FormatStringSpecifier::DECIMAL_PLACES) {
        ++i;
        obj.mx_decimal_places = FormatStringObject::get_number(fmt_str, i);
      }

      // store type specifier
      switch (fmt_str[i]) {
        case FormatStringSpecifier::INT :
          obj.type = FormatStringSpecifier::INT;
          break;
        case FormatStringSpecifier::FLOAT :
          obj.type = FormatStringSpecifier::FLOAT;
          break;
        case FormatStringSpecifier::STRING :
          obj.type = FormatStringSpecifier::STRING;
          break;
        case FormatStringSpecifier::OBJECT :
          obj.type = FormatStringSpecifier::OBJECT;
          break;
        case FormatStringSpecifier::CHAR:
          obj.type = FormatStringSpecifier::CHAR;
          break;
        case FormatStringSpecifier::BOOL:
          obj.type = FormatStringSpecifier::BOOL;
          break;
        default:
          std::cerr << "Error: Unknown format specifier '"
                    << fmt_str[i] << "'\n";
          std::exit(1);
      }
      ++i;

      // store right-pad character (if it was specified, comes after '<')
      if (fmt_str[i] == FormatStringSpecifier::PAD_RIGHT) {
        ++i;
        obj.right_pad_chr = fmt_str[i++];
      }

      assert(fmt_str[i] == FormatStringSpecifier::CLOSE);

      obj.end_idx = i + 1;
      fmt_objs.push_back(obj);

      if (fmt_str[i] != FormatStringSpecifier::CLOSE) {
        std::cerr << "Error: Expected '" << FormatStringSpecifier::CLOSE
                  << "', found '" << fmt_str[i] << "'!\n";
        std::exit(1);
      }
    } else {
      ++i;
    }
  }

  return fmt_objs;
}

// handle max length and padding of to-be-printed format argument
template<class LogImpl>
inline void Logger<LogImpl>::_pad_fmt_arg(std::ostream &str,
  const std::string &arg, const FormatStringObject &fmt_obj) {
  size_t str_len = arg.size();       // length of to-be-printed string arg
  uint16_t mx_len = fmt_obj.mx_len;  // max length of to-be-printed arg
  if (mx_len) {
    if (mx_len < str_len) {
      // no space for padding -> just print string until max is reached
      for (uint16_t i = 0; i < mx_len; ++i) str << arg[i];
    } else {
      // if left- AND right-padding was specified in the format,
      // distribute the padding equally between left and right
      // (left will be preferred if the max padding number is uneven)
      uint16_t mx_padding = mx_len - str_len;
      uint16_t mx_right_padding = mx_padding / 2;
      uint16_t mx_left_padding = mx_padding - mx_right_padding;

      mx_left_padding = mx_padding > 0 && fmt_obj.right_pad_chr != '\0' ?
                          mx_left_padding: mx_padding;
      mx_right_padding = mx_padding > 0 && fmt_obj.left_pad_chr != '\0' ?
                          mx_right_padding : mx_padding;

      // left-pad (no-op if mx_left_padding<0 or no pad char was specified)
      if (fmt_obj.left_pad_chr != '\0') {
        FormatStringObject::pad(str, mx_left_padding, fmt_obj.left_pad_chr);
      }

      // print entire argument
      str << arg;

      // right-pad (no-op if mx_right_padding<0 or no pad char was specified)
      if (fmt_obj.right_pad_chr != '\0') {
        FormatStringObject::pad(str, mx_right_padding, fmt_obj.right_pad_chr);
      }
    }
  } else {
    str << arg;
  }
}

template<class LogImpl>
template<typename T>
inline void Logger<LogImpl>::_log_fmt_arg(std::stringstream &fmt_arg, T &&_arg,
  const char *log_fmt, int start_idx, int end_idx,
  const FormatStringObject &fmt_obj) {
  // log the regular string part before the current format specifier
  for (int i = start_idx; i < end_idx; ++i) fmt_arg << log_fmt[i];

  std::stringstream tmp;
  tmp << _arg;
  std::string arg = tmp.str();

  switch (fmt_obj.type) {
    case FormatStringSpecifier::INT :
      _pad_fmt_arg(fmt_arg, arg, fmt_obj);
      break;
    case FormatStringSpecifier::FLOAT :
    {
      // convert floating-point number to string and check if the
      // number of decimal places matches the max number of decimal
      // places specified in the format; if not, cut the overflow off
      size_t dot_idx = arg.find('.');
      assert(dot_idx != arg.npos);

      size_t n_decimal_places = arg.size() - dot_idx - 1;

      if (n_decimal_places > fmt_obj.mx_decimal_places) {
        arg = arg.substr(0, dot_idx + 1 + fmt_obj.mx_decimal_places);
      }

      _pad_fmt_arg(fmt_arg, arg, fmt_obj);
      break;
    }
    case FormatStringSpecifier::STRING :
      _pad_fmt_arg(fmt_arg, arg, fmt_obj);
      break;
    case FormatStringSpecifier::OBJECT :
      _pad_fmt_arg(fmt_arg, arg, fmt_obj);
      break;
    case FormatStringSpecifier::CHAR :
      _pad_fmt_arg(fmt_arg, arg, fmt_obj);
      break;
    case FormatStringSpecifier::BOOL :
    {
      std::string bool_val = (arg == "1") ? "true" : "false";
      _pad_fmt_arg(fmt_arg, bool_val, fmt_obj);
      break;
    }
    default:
    fmt_arg << "Unknown type";
  }
}

// terminate the variadic argument recursion and print the rest of
// the text in the format string after the last format specifier
template<class LogImpl>
inline void Logger<LogImpl>::_log_format_string_args(
  std::stringstream &fmt_stream,
  const std::vector<FormatStringObject> &fmt_objs,
  const char *fmt_str, int start_idx, int end_idx, int obj_idx) {
  for (int i = start_idx; i < end_idx; ++i) fmt_stream << fmt_str[i];
}

template<class LogImpl>
template<typename T, typename ...Tr>
inline void Logger<LogImpl>::_log_format_string_args(
  std::stringstream &fmt_stream,
  const std::vector<FormatStringObject> &fmt_objs, const char *fmt_str,
  int start_idx, int end_idx, int obj_idx, T &&first, Tr &&...rest) {
  _log_fmt_arg(fmt_stream, std::move(first), fmt_str,
              start_idx, end_idx, fmt_objs[obj_idx]);

  // case where recursion anchor is called
  if (obj_idx + 1 >= fmt_objs.size()) {
    _log_format_string_args(fmt_stream, fmt_objs, fmt_str,
                            fmt_objs[obj_idx].end_idx,
                            std::strlen(fmt_str), obj_idx,
                            std::forward<Tr>(rest)...);
  } else {
    _log_format_string_args(fmt_stream, fmt_objs, fmt_str,
                            fmt_objs[obj_idx].end_idx,
                            fmt_objs[obj_idx + 1].start_idx, obj_idx + 1,
                            std::forward<Tr>(rest)...);
  }
}

// create a new Logger object and transfer ownership
// of the Logger to the binding variable
template<class LogImpl>
inline Logger<LogImpl>* create_log(const char *name, const LogImpl &log_impl) {
  return new Logger(name, log_impl);
}

// create a new Logger object with a custom LogImpl class
// and transfer ownership of the Logger to the binding variable;
// the Logger will take ownershio of the LogImpl object and delete
// it when it gets deleted itself
template<class LogImpl>
inline Logger<LogImpl>* create_log(const char *name, LogLevel lvl,
  LogFormat fmt, const LogImpl &log_impl) {
  return new Logger(name, lvl, fmt, log_impl);
}

}  // namespace cpplog

#endif  // CPPLOG_LOGGER_H_
