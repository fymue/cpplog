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

#ifndef CPPLOG_H_
#define CPPLOG_H_

#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <cassert>

// ### types that can be logged by default: ###
// all primitive types
#include <string>
#include <vector>
#include <array>
#include <map>
#include <utility>  // for std::pair
#include <unordered_map>
// ############################################

namespace cpplog {

// available format flags for a log message
enum LogFmt {
  NEWLINE          = 1 << 0,  // append newline at the end of the log msg
  TIMESTAMP        = 1 << 1,  // add system time at the beginning of log msg
  HIGHLIGHT_RED    = 1 << 2,  // highlight the log msg red
  HIGHLIGHT_GREEN  = 1 << 3,  // highlight the log msg green
  HIGHLIGHT_YELLOW = 1 << 4,  // highlight the log msg yellow
  HIGHLIGHT_DEF    = 1 << 5,  // use default terminal color for log msg
  VERBOSE          = 1 << 6,  // print entire content (for container-like)
  TYPE_SIZE        = 1 << 7,  // print (estimated) size of type
  NAME             = 1 << 8,  // print name of Logger
};

// available log levels (also compliant with a valid log format)
enum Level {
  STANDARD = NEWLINE | TIMESTAMP,
  DEBUG    = NEWLINE | TIMESTAMP | TYPE_SIZE | NAME
};

typedef uint64_t LogFormat;

// ANSI color codes for colorful logging
static const char *__ansi_red     = "\033[31m";
static const char *__ansi_green   = "\033[32m";
static const char *__ansi_yellow  = "\033[33m";
static const char *__ansi_default = "\033[39m";

// limit number of chars of logged string
static constexpr int CPPLOG_MX_STR_LEN = 50;

// limit number of elements of logged container
static constexpr int CPPLOG_MX_ELS     = 10;

// ### operator<< overloads for most std library types ###

template<typename T>
static std::ostream &operator<<(std::ostream &stream,
                                const std::vector<T> &vec) {
  if (vec.empty()) {
    stream << "vector: [] ";
    return stream;
  }

  stream << "vector: [" << vec[0];
  for (size_t i = 1; i < vec.size(); ++i) {
    stream << ", " << vec[i];
  }
  stream << "] ";

  return stream;
}

template<typename T, size_t SIZE>
static std::ostream &operator<<(std::ostream &stream,
                                const std::array<T, SIZE> &arr) {
  if (arr.empty()) {
    stream << "array: [] ";
    return stream;
  }

  stream << "array: [" << arr[0];
  for (size_t i = 1; i < arr.size(); ++i) {
    stream << ", " << arr[i];
  }
  stream << "] ";

  return stream;
}

template<typename K, typename V>
static std::ostream &operator<<(std::ostream &stream,
                                const std::map<K, V> &map) {
  if (map.empty()) {
    stream << "map: {} ";
    return stream;
  }

  stream << "map: {";
  auto map_it = map.cbegin();
  stream << map_it->first << ": " << map_it->second;
  ++map_it;

  for (; map_it != map.cend(); ++map_it) {
    stream << ", " << map_it->first << ": " << map_it->second;
  }

  stream << "} ";

  return stream;
}

template<typename K, typename V>
static std::ostream &operator<<(std::ostream &stream,
                                const std::unordered_map<K, V> &map) {
  if (map.empty()) {
    stream << "unordered_map: {} ";
    return stream;
  }

  stream << "unordered_map: {";
  auto map_it = map.cbegin();
  stream << map_it->first << ": " << map_it->second;
  ++map_it;

  for (; map_it != map.cend(); ++map_it) {
    stream << ", " << map_it->first << ": " << map_it->second;
  }

  stream << "} ";

  return stream;
}

template<typename T, typename U>
static std::ostream &operator<<(std::ostream &stream,
                                const std::pair<T, U> &pair) {
  stream << "pair: {" << pair.first << ", " << pair.second << "} ";
  return stream;
}

// ##########################################################

/*
 * specifies how a certain datatype should be logged;
 * defines a "void log(std::ostream &stream, CustomType t, LogFormat fmt)"
 * method for every loggable type;
 * this class should be extend if you wish to add log definitions
 * for you own custom types as the main "Logger" class will always
 * call a "log" method of whatever LogImpl object it owns
 */
class LoggerImpl {
 private:
  // buffer for current time
  char time_str[sizeof("hh:mm:ss")];

  // start time of program
  // (will be used to calculate current time whenever a timestamp is logged)
  std::time_t start_time;

  // name of Logger (for easier differentiation when using multiple Loggers)
  std::string name;

 public:
  LoggerImpl() :
    start_time(std::time(nullptr)), name("") {}

  void set_name(const std::string &_name) {
    name = _name;
  }

  /*
   * initialize the log msg based on the log format options;
   * the passed type needs to have an operator<< overload and
   * will be passed directly into the stream
   */
  template<typename T>
  void parse_fmt_opts(std::ostream &stream, const T &t,
                      LogFormat fmt, size_t type_size = 0) {
    // set color of text
    if (fmt & LogFmt::HIGHLIGHT_GREEN) {
      stream << __ansi_green;
    } else if (fmt & LogFmt::HIGHLIGHT_YELLOW) {
      stream << __ansi_yellow;
    } else if (fmt & LogFmt::HIGHLIGHT_RED) {
      stream << __ansi_red;
    } else if (fmt & LogFmt::HIGHLIGHT_DEF) {
      stream << __ansi_default;
    }

    bool log_name      = fmt & LogFmt::NAME;
    bool log_timestamp = fmt & LogFmt::TIMESTAMP;

    std::strftime(time_str, sizeof(time_str),
                  "%T", std::localtime(&start_time));

    if (log_name && log_timestamp) {
      stream << "[" << name << ", " << time_str << "] ";
    } else {
      if (fmt & LogFmt::NAME) {
        stream << "[" << name << "] ";
      }
      if (fmt & LogFmt::TIMESTAMP) {
        stream << "[" << time_str << "] ";
      }
    }

    // add type to stream (without any special formatting)
    stream << t;

    // print estimtated size of type (in bytes)
    if (fmt & LogFmt::TYPE_SIZE) {
      stream << " (SIZE ~= ";
      if (type_size) {
        stream << type_size;
      } else {
        stream << sizeof(t);
      }
      stream << " bytes)";
    }

    // reset to default colors again
    stream << __ansi_default;

    if (fmt & LogFmt::NEWLINE) {
      stream << '\n';
    }
  }

  // log (unsigned) integer types
  void log(std::ostream &stream, int x, LogFormat fmt) {
    parse_fmt_opts(stream, x, fmt);
  }

  // log floating point types
  void log(std::ostream &stream, double x, LogFormat fmt) {
    parse_fmt_opts(stream, x, fmt);
  }

  // log characters
  void log(std::ostream &stream, char c, LogFormat fmt) {
    parse_fmt_opts(stream, c, fmt);
  }

  // log booleans
  void log(std::ostream &stream, bool b, LogFormat fmt) {
    const char *bool_val = b ? "true" : "false";
    parse_fmt_opts(stream, bool_val, fmt, 1);
  }

  // log std::pair
  template<typename T, typename U>
  void log(std::ostream &stream, const std::pair<T, U> &p, LogFormat fmt) {
    parse_fmt_opts(stream, p, fmt);
  }

  // log C string
  void log(std::ostream &stream, const char *str, LogFormat fmt) {
    size_t str_len = std::strlen(str);

    if (fmt & LogFmt::VERBOSE || str_len < CPPLOG_MX_STR_LEN) {
      parse_fmt_opts(stream, str, fmt, str_len);
    } else {
      // if a string is longer than 20 characters,
      // print shortened version of it
      char formatted_str[64];
      int pos = std::snprintf(formatted_str,
                              sizeof(formatted_str), "String: \"");

      int mx_border = 8;
      for (int i = 0; i < mx_border; ++i, ++pos) {
        formatted_str[pos] = str[i];
      }

      formatted_str[pos++] = '.';
      formatted_str[pos++] = '.';
      formatted_str[pos++] = '.';
      formatted_str[pos++] = ' ';

      for (size_t i = str_len - mx_border; i < str_len; ++i, ++pos) {
        formatted_str[pos] = str[i];
      }

      formatted_str[pos++] = '"';
      formatted_str[pos] = '\0';

      parse_fmt_opts(stream, formatted_str, fmt);
    }
  }

  // log std::string
  void log(std::ostream &stream, const std::string &str, LogFormat fmt) {
    log(stream, str.c_str(), fmt);
  }

  // log std::vector
  template<typename T>
  void log(std::ostream &stream, const std::vector<T> &vec, LogFormat fmt) {
    size_t size = vec.size();
    size_t size_in_bytes = size * sizeof(T);

    if (fmt & LogFmt::VERBOSE || size < CPPLOG_MX_ELS) {
      parse_fmt_opts(stream, vec, fmt, size_in_bytes);
    } else {
      // if a vector contains more than mx_size elements,
      // print shortened version of it
      std::stringstream formatted_vec;
      formatted_vec << "vector: [";
      formatted_vec << vec[0];

      for (size_t i = 1; i < CPPLOG_MX_ELS / 2; ++i) {
        formatted_vec << ", " << vec[i];
      }

      formatted_vec << " ... ";
      size_t start = size - (CPPLOG_MX_ELS / 2);
      formatted_vec << vec[start++];

      for (size_t i = start; i < size; ++i) {
        formatted_vec << ", " << vec[i];
      }
      formatted_vec << "] ";

      parse_fmt_opts(stream, formatted_vec.rdbuf(), fmt, size_in_bytes);
    }
  }

  // log std::array
  template<typename T, size_t SIZE>
  void log(std::ostream &stream,
           const std::array<T, SIZE> &arr, LogFormat fmt) {
    size_t size = arr.size();
    size_t size_in_bytes = size * sizeof(T);

    if (fmt & LogFmt::VERBOSE || size < CPPLOG_MX_ELS) {
      parse_fmt_opts(stream, arr, fmt, size_in_bytes);
    } else {
      // if an array contains more than mx_size elements,
      // print shortened version of it
      std::stringstream formatted_arr;
      formatted_arr << "array: [";
      formatted_arr << arr[0];

      for (size_t i = 1; i < CPPLOG_MX_ELS / 2; ++i) {
        formatted_arr << ", " << arr[i];
      }

      formatted_arr << " ... ";
      size_t start = size - (CPPLOG_MX_ELS / 2);
      formatted_arr << arr[start++];

      for (size_t i = start; i < size; ++i) {
        formatted_arr << ", " << arr[i];
      }
      formatted_arr << "] ";

      parse_fmt_opts(stream, formatted_arr.rdbuf(), fmt, size_in_bytes);
    }
  }

  // log std::map
  template<typename K, typename V>
  void log(std::ostream &stream, const std::map<K, V> &map, LogFormat fmt) {
    size_t size = map.size();
    size_t size_in_bytes = size * sizeof(K) + size * sizeof(V);  // estimate

    if (fmt & LogFmt::VERBOSE || size < CPPLOG_MX_ELS) {
      parse_fmt_opts(stream, map, fmt, size_in_bytes);
    } else {
      // if a map contains more than mx_size key-value pairs,
      // print shortened version of it
      std::stringstream formatted_map;
      auto map_it = map.cbegin();
      size_t left_start = 1;
      formatted_map << "map: {";
      formatted_map << map_it->first << ": " << map_it->second;
      ++map_it;

      for (; left_start < CPPLOG_MX_ELS / 2; ++map_it, ++left_start) {
        formatted_map << ", " << map_it->first << ": " << map_it->second;
      }

      formatted_map << " ... ";
      size_t skip_c = (size - (CPPLOG_MX_ELS / 2)) - left_start;
      std::advance(map_it, skip_c);
      left_start += skip_c;

      formatted_map << map_it->first << ": " << map_it->second;
      ++map_it;
      ++left_start;

      for (; left_start < size; ++map_it, ++left_start) {
        formatted_map << ", " << map_it->first << ": " << map_it->second;
      }
      formatted_map << "} ";

      parse_fmt_opts(stream, formatted_map.rdbuf(), fmt, size_in_bytes);
    }
  }

  // log std::unordered_map
  template<typename K, typename V>
  void log(std::ostream &stream,
           const std::unordered_map<K, V> &map, LogFormat fmt) {
    size_t size = map.size();
    size_t size_in_bytes = size * sizeof(K) + size * sizeof(V);  // estimate

    if (fmt & LogFmt::VERBOSE || size < CPPLOG_MX_ELS) {
      parse_fmt_opts(stream, map, fmt, size_in_bytes);
    } else {
      // if an unordered map contains more than mx_size key-value pairs,
      // print shortened version of it
      std::stringstream formatted_map;
      auto map_it = map.cbegin();
      size_t left_start = 1;
      formatted_map << "unordered_map: {";
      formatted_map << map_it->first << ": " << map_it->second;
      ++map_it;

      for (; left_start < CPPLOG_MX_ELS / 2; ++map_it, ++left_start) {
        formatted_map << ", " << map_it->first << ": " << map_it->second;
      }

      formatted_map << " ... ";
      size_t skip_c = (size - (CPPLOG_MX_ELS / 2)) - left_start;
      std::advance(map_it, skip_c);
      left_start += skip_c;

      formatted_map << map_it->first << ": " << map_it->second;
      ++map_it;
      ++left_start;

      for (; left_start < size; ++map_it, ++left_start) {
        formatted_map << ", " << map_it->first << ": " << map_it->second;
      }
      formatted_map << "} ";

      parse_fmt_opts(stream, formatted_map.rdbuf(), fmt, size_in_bytes);
    }
  }
};

/*
 * The Logger class contains a reference to std::cerr, which
 * is the output stream for all logged messages;
 * you can specify the log level and log format of the
 * to-be-logged messages using the set_log_level and
 * set_log_format methods; this class contains methods that
 * support the logging of all primitive and most std library types;
 * if you wish to log your own custom types, please extend the
 * "LoggerImpl" class and add your own
 * "void log(std::ostream &stream, YourCustomType t, LogFormat fmt)"
 * definitions that specify how your custom types should be logged
 * and provide your extension of the LoggerImpl class as the template
 * class for the Logger class;
 * the Logger class will take ownership of the LoggerImpl object,
 * so be aware that it will be deleted whenever the Logger class
 * get deleted
 */
template<class LogImpl = LoggerImpl>
class Logger {
 private:
  Level _log_lvl;
  std::ostream &_stream;
  std::string _name;
  LogFormat _log_format;
  LogImpl *_log_impl;
  std::mutex _mutex;

  // default log formats for error, warning and info messages
  static const LogFormat _default_err_fmt =
    LogFmt::HIGHLIGHT_RED | LogFmt::TIMESTAMP | LogFmt::NEWLINE;
  static const LogFormat _default_warn_fmt =
    LogFmt::HIGHLIGHT_YELLOW | LogFmt::TIMESTAMP | LogFmt::NEWLINE;
  static const LogFormat _default_info_fmt =
    LogFmt::HIGHLIGHT_GREEN | LogFmt::TIMESTAMP | LogFmt::NEWLINE;

  /*
   * contains data of a to-be-formatted object from the format string;
   * the format string can parse decimal numbers, floating point numbers,
   * strings, objects and characters. Formatting can be specified in curly brackets
   * with Python/printf-like syntax:
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
   *   - 'd' | 'f' | 's' | 'o': specify that the to-be-printed parameter
   *     is a decimal number, floating-point number, string or object
   *   - '<' : any character put after '<' will be used to right-pad
   *     the to-be-printed type (if a max character size was also specified);
   *     '>' should be the 2nd to last character in the specified format
   */
  struct FormatStringObject {
    enum FORMAT {
     NONE           = '\0',
     INT            = 'd',
     FLOAT          = 'f',
     STRING         = 's',
     CHAR           = 'c',
     BOOL           = 'b',
     OBJECT         = 'o',
     OPEN           = '{',
     CLOSE          = '}',
     DECIMAL_PLACES = '.',
     PAD_LEFT       = '>',
     PAD_RIGHT      = '<'
    };

    FORMAT type;
    char left_pad_chr, right_pad_chr;
    uint16_t mx_len, mx_decimal_places;

    // start and end idx of the format specifier in the format string
    int start_idx, end_idx;

    FormatStringObject() :
      type(NONE), left_pad_chr('\0'), right_pad_chr('\0'),
      mx_len(0), mx_decimal_places(0), start_idx(0), end_idx(0) {}

    static void pad(std::ostream &stream, int n, char pad_chr) {
      if (n < 0) return;

      for (int i = 0; i < n; ++i) {
        stream << pad_chr;
      }
    }

    static bool is_number(char chr) {
      if (chr < '0' || chr > '9') {
        return false;
      }

      return true;
    }

    static std::string get_type(FORMAT type) {
      switch (type) {
        case FORMAT::INT    : return std::string("Decimal number");
        case FORMAT::FLOAT  : return std::string("Floating-point number");
        case FORMAT::STRING : return std::string("String");
        case FORMAT::OBJECT : return std::string("Object");
        case FORMAT::CHAR   : return std::string("Character");
        case FORMAT::BOOL   : return std::string("Boolean");
        case FORMAT::NONE   : return std::string("None");
      }
    }

    // parse a (multi-digit) number from format string
    static uint16_t get_number(const char *fmt_str, int &fmt_str_idx) {
      constexpr uint8_t MX_DIGITS = 3;
      char number[] = "0\0\0";
      int number_i = 0;

      while (number_i < MX_DIGITS &&
             FormatStringObject::is_number(fmt_str[fmt_str_idx])) {
        number[number_i++] = fmt_str[fmt_str_idx++];
      }

      return static_cast<uint16_t>(std::stoi(number));
    }

    friend std::ostream &operator<<(std::ostream &stream,
                                    const FormatStringObject &obj) {
      stream << "Type: " << FormatStringObject::get_type(obj.type)
             << ", Left pad: '" << obj.left_pad_chr
             << "', Right pad: '" << obj.right_pad_chr << "', Mx Length: "
             << obj.mx_len << ", Mx decimal places: " << obj.mx_decimal_places
             << "\n";
      return stream;
    }
  };

  std::vector<FormatStringObject> _parse_format_string(const char *fmt_str) {
    std::vector<FormatStringObject> fmt_objs;

    size_t str_len = std::strlen(fmt_str);

    for (int i = 0; i < str_len;) {
      if (fmt_str[i] == FormatStringObject::OPEN) {
        FormatStringObject obj;
        obj.start_idx = i;
        ++i;

        // store left pad character (if specified, comes before '>')
        if (fmt_str[i + 1] == FormatStringObject::PAD_LEFT) {
          obj.left_pad_chr = fmt_str[i];
          i += 2;
        }

        // store max number of characters of formatted argument
        obj.mx_len = FormatStringObject::get_number(fmt_str, i);

        // store max number of decimal places after floating point number
        if (fmt_str[i] == FormatStringObject::DECIMAL_PLACES) {
          ++i;
          obj.mx_decimal_places = FormatStringObject::get_number(fmt_str, i);
        }

        // store type specifier
        switch (fmt_str[i]) {
          case FormatStringObject::INT :
            obj.type = FormatStringObject::INT;
            break;
          case FormatStringObject::FLOAT :
            obj.type = FormatStringObject::FLOAT;
            break;
          case FormatStringObject::STRING :
            obj.type = FormatStringObject::STRING;
            break;
          case FormatStringObject::OBJECT :
            obj.type = FormatStringObject::OBJECT;
            break;
          case FormatStringObject::CHAR:
            obj.type = FormatStringObject::CHAR;
            break;
          case FormatStringObject::BOOL:
            obj.type = FormatStringObject::BOOL;
            break;
          default:
            std::cerr << "Error: Unknown format specifier '"
                      << fmt_str[i] << "'\n";
            std::exit(1);
        }
        ++i;

        // store right-pad character (if it was specified, comes after '<')
        if (fmt_str[i] == FormatStringObject::PAD_RIGHT) {
          ++i;
          obj.right_pad_chr = fmt_str[i++];
        }

        assert(fmt_str[i] == FormatStringObject::CLOSE);

        obj.end_idx = i + 1;
        fmt_objs.push_back(obj);

        if (fmt_str[i] != FormatStringObject::CLOSE) {
          std::cerr << "Error: Expected '" << FormatStringObject::CLOSE
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
  void _pad_fmt_arg(std::ostream &str, const std::string &arg,
                    const FormatStringObject &fmt_obj) {
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

  template<typename T>
  void _log_fmt_arg(std::stringstream &fmt_arg, T &&_arg,
                    const char *log_fmt, int start_idx, int end_idx,
                    const FormatStringObject &fmt_obj) {
    // log the regular string part before the current format specifier
    for (int i = start_idx; i < end_idx; ++i) fmt_arg << log_fmt[i];

    std::stringstream tmp;
    tmp << _arg;
    std::string arg = tmp.str();

    switch (fmt_obj.type) {
      case FormatStringObject::INT :
        _pad_fmt_arg(fmt_arg, arg, fmt_obj);
        break;
      case FormatStringObject::FLOAT :
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
      case FormatStringObject::STRING :
        _pad_fmt_arg(fmt_arg, arg, fmt_obj);
        break;
      case FormatStringObject::OBJECT :
        _pad_fmt_arg(fmt_arg, arg, fmt_obj);
        break;
      case FormatStringObject::CHAR :
        _pad_fmt_arg(fmt_arg, arg, fmt_obj);
        break;
      case FormatStringObject::BOOL :
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
  void _log_format_string_args(std::stringstream &fmt_stream,
                               const std::vector<FormatStringObject> &fmt_objs,
                               const char *fmt_str,
                               int start_idx, int end_idx, int obj_idx) {
    for (int i = start_idx; i < end_idx; ++i) fmt_stream << fmt_str[i];
  }

  template<typename T, typename ...Tr>
  void _log_format_string_args(std::stringstream &fmt_stream,
                               const std::vector<FormatStringObject> &fmt_objs,
                               const char *fmt_str, int start_idx, int end_idx,
                               int obj_idx, T &&first, Tr &&...rest) {
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

 public:
  Logger() :
    _stream(std::cerr), _name("LOG") {
    set_log_level(Level::STANDARD);
    set_log_format(Level::STANDARD);
    set_log_impl(nullptr);
  }

  Logger(const char *name, LogImpl *log_impl = nullptr) :
    _stream(std::cerr), _name(name) {
    set_log_level(Level::STANDARD);
    set_log_format(Level::STANDARD);
    set_log_impl(log_impl);
  }

  Logger(const char *name, Level lvl,
         LogFormat fmt, LogImpl *log_impl = nullptr) :
    _log_lvl(lvl), _stream(std::cerr), _name(name), _log_format(fmt) {
    set_log_level(lvl);
    set_log_format(fmt);
    set_log_impl(log_impl);
  }

  ~Logger() {
    delete _log_impl;
  }

  // set log level
  void set_log_level(Level lvl) {
    _log_lvl = lvl;
    _log_format = _log_lvl;
  }

  // specify log format (see LogFmt enum for available options)
  void set_log_format(LogFormat fmt) {
    _log_format = fmt;
  }

  /*
   * set the log implementation object;
   * the Logger class will take ownership of the LogImpl object,
   * so be aware that it will be deleted whenever the Logger class
   * get deleted
   */
  void set_log_impl(LogImpl *log_impl) {
    // if log implementation already exists, delete it first
    if (_log_impl) {
      delete _log_impl;
    }

    // if a valid ptr was passed, simply take ownership;
    // else, create a new log implementation object
    if (log_impl) {
      _log_impl = log_impl;
    } else {
      _log_impl = new LogImpl();
    }

    _log_impl->set_name(_name);
  }

  template<typename T>
  void error(const T &t, LogFormat fmt) {
    std::lock_guard<std::mutex> lock(_mutex);
    _log_impl->log(_stream, t, fmt | _default_err_fmt);
  }

  template<typename T>
  void error(const T &t) {
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
  template<typename ...T>
  void error(const char *fmt_str, T&&... args) {
    error(fmt_str, _log_format, std::forward<T>(args)...);
  }

  template<typename T, typename ...Tr>
  void error(const char *fmt_str, LogFormat fmt, T &&first, Tr&&... args) {
    std::vector<FormatStringObject> objs = _parse_format_string(fmt_str);
    std::stringstream fmt_stream;
    _log_format_string_args(fmt_stream, objs, fmt_str, 0,
                            objs.front().start_idx, 0, std::move(first),
                            std::forward<Tr>(args)...);
    std::lock_guard<std::mutex> lock(_mutex);
    _log_impl->parse_fmt_opts(_stream, fmt_stream.rdbuf(),
                              fmt | _default_err_fmt);
  }

  template<typename T>
  void warn(const T &t, LogFormat fmt) {
    warn(t, _log_format);
  }

  template<typename T>
  void warn(const T &t) {
    std::lock_guard<std::mutex> lock(_mutex);
    _log_impl->log(_stream, t, _log_format | _default_warn_fmt);
  }

  /*
   * print args accoring to format specified by fmt_str;
   * the format string can parse decimal numbers, floating point numbers,
   * strings, objects and characters. Formatting can be specified in curly
   * brackets with Python/printf-like syntax:
   *   {_>10.2f} -> left-padded w/ '_', 10 chars, 2 decimal place float
   *   {0>8d< }   -> left-padded w/ '0, 8 chars, decimal, right-padded w/ ' '
   */
  template<typename ...T>
  void warn(const char *fmt_str, T&&... args) {
    warn(fmt_str, _log_format, std::forward<T>(args)...);
  }

  template<typename T, typename ...Tr>
  void warn(const char *fmt_str, LogFormat fmt, T &&first, Tr&&... args) {
    std::vector<FormatStringObject> objs = _parse_format_string(fmt_str);
    std::stringstream fmt_stream;
    _log_format_string_args(fmt_stream, objs, fmt_str, 0,
                            objs.front().start_idx, 0, std::move(first),
                            std::forward<Tr>(args)...);
    std::lock_guard<std::mutex> lock(_mutex);
    _log_impl->parse_fmt_opts(_stream, fmt_stream.rdbuf(),
                              fmt | _default_warn_fmt);
  }

  template<typename T>
  void info(const T &t, LogFormat fmt) {
    std::lock_guard<std::mutex> lock(_mutex);
    _log_impl->log(_stream, t, fmt | _default_info_fmt);
  }

  template<typename T>
  void info(const T &t) {
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
  template<typename ...T>
  void info(const char *fmt_str, T&&... args) {
    info(fmt_str, _log_format, std::forward<T>(args)...);
  }

  template<typename T, typename ...Tr>
  void info(const char *fmt_str, LogFormat fmt, T &&first, Tr&&... args) {
    std::vector<FormatStringObject> objs = _parse_format_string(fmt_str);
    std::stringstream fmt_stream;
    _log_format_string_args(fmt_stream, objs, fmt_str, 0,
                            objs.front().start_idx, 0, std::move(first),
                            std::forward<Tr>(args)...);
    std::lock_guard<std::mutex> lock(_mutex);
    _log_impl->parse_fmt_opts(_stream, fmt_stream.rdbuf(),
                              fmt | _default_info_fmt);
  }
};

// create a new Logger object and transfer ownership
// of the Logger to the binding variable
template<class LogImpl = LoggerImpl>
inline Logger<LogImpl>* create_log(const char *name,
                                   LogImpl *log_impl = nullptr) {
  return new Logger(name, log_impl);
}

// create a new Logger object with a custom LogImpl class
// and transfer ownership of the Logger to the binding variable;
// the Logger will take ownershio of the LogImpl object and delete
// it when it gets deleted itself
template<class LogImpl = LoggerImpl>
inline Logger<LogImpl>* create_log(const char *name, Level lvl,
                                   LogFormat fmt, LogImpl *log_impl) {
  return new Logger(name, lvl, fmt, log_impl);
}

}  // namespace cpplog

#endif  // CPPLOG_H_
