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

#ifndef LOG_H_
#define LOG_H_

#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <ctime>

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

  // log std::pair
  template<typename T, typename U>
  void log(std::ostream &stream, const std::pair<T, U> &p, LogFormat fmt) {
    parse_fmt_opts(stream, p, fmt);
  }

  // log C strings and std::string
  void log(std::ostream &stream, const std::string &str, LogFormat fmt) {
    size_t str_len = str.size();

    if (fmt & LogFmt::VERBOSE || str_len < 20) {
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

  // log std::vector
  template<typename T>
  void log(std::ostream &stream, const std::vector<T> &vec, LogFormat fmt) {
    size_t size = vec.size();
    size_t mx_size = 10;
    size_t size_in_bytes = size * sizeof(T);

    if (fmt & LogFmt::VERBOSE || size < mx_size) {
      parse_fmt_opts(stream, vec, fmt, size_in_bytes);
    } else {
      // if a vector contains more than mx_size elements,
      // print shortened version of it
      std::stringstream formatted_vec;
      formatted_vec << "vector: [";
      formatted_vec << vec[0];

      for (size_t i = 1; i < mx_size / 2; ++i) {
        formatted_vec << ", " << vec[i];
      }

      formatted_vec << " ... ";
      size_t start = size - (mx_size / 2);
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
    size_t mx_size = 10;
    size_t size_in_bytes = size * sizeof(T);

    if (fmt & LogFmt::VERBOSE || size < mx_size) {
      parse_fmt_opts(stream, arr, fmt, size_in_bytes);
    } else {
      // if an array contains more than mx_size elements,
      // print shortened version of it
      std::stringstream formatted_arr;
      formatted_arr << "array: [";
      formatted_arr << arr[0];

      for (size_t i = 1; i < mx_size / 2; ++i) {
        formatted_arr << ", " << arr[i];
      }

      formatted_arr << " ... ";
      size_t start = size - (mx_size / 2);
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
    size_t mx_size = 8;
    size_t size_in_bytes = size * sizeof(K) + size * sizeof(V);  // estimate

    if (fmt & LogFmt::VERBOSE || size < mx_size) {
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

      for (; left_start < mx_size / 2; ++map_it, ++left_start) {
        formatted_map << ", " << map_it->first << ": " << map_it->second;
      }

      formatted_map << " ... ";
      size_t skip_c = (size - (mx_size / 2)) - left_start;
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
    size_t mx_size = 8;
    size_t size_in_bytes = size * sizeof(K) + size * sizeof(V);  // estimate

    if (fmt & LogFmt::VERBOSE || size < mx_size) {
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

      for (; left_start < mx_size / 2; ++map_it, ++left_start) {
        formatted_map << ", " << map_it->first << ": " << map_it->second;
      }

      formatted_map << " ... ";
      size_t skip_c = (size - (mx_size / 2)) - left_start;
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

  // default log formats for error, warning and info messages
  static const LogFormat _default_err_fmt =
    LogFmt::HIGHLIGHT_RED | LogFmt::TIMESTAMP | LogFmt::NEWLINE;
  static const LogFormat _default_warn_fmt =
    LogFmt::HIGHLIGHT_YELLOW | LogFmt::TIMESTAMP | LogFmt::NEWLINE;
  static const LogFormat _default_info_fmt =
    LogFmt::HIGHLIGHT_GREEN | LogFmt::TIMESTAMP | LogFmt::NEWLINE;

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
    _log_impl->log(_stream, t, fmt | _default_err_fmt);
  }

  template<typename T>
  void error(const T &t) {
    _log_impl->log(_stream, t, _log_format | _default_err_fmt);
  }

  template<typename T>
  void warn(const T &t, LogFormat fmt) {
    _log_impl->log(_stream, t, fmt | _default_warn_fmt);
  }

  template<typename T>
  void warn(const T &t) {
    _log_impl->log(_stream, t, _log_format | _default_warn_fmt);
  }

  template<typename T>
  void info(const T &t, LogFormat fmt) {
    _log_impl->log(_stream, t, fmt | _default_info_fmt);
  }

  template<typename T>
  void info(const T &t) {
    _log_impl->log(_stream, t, _log_format | _default_info_fmt);
  }
};

template<class LogImpl = LoggerImpl>
inline Logger<LogImpl>* create_log(const char *name,
                                   LogImpl *log_impl = nullptr) {
  return new Logger(name, log_impl);
}

template<class LogImpl = LoggerImpl>
inline Logger<LogImpl>* create_log(const char *name, Level lvl,
                                   LogFormat fmt, LogImpl *log_impl) {
  return new Logger(name, lvl, fmt, log_impl);
}

}  // namespace cpplog

#endif  // LOG_H_
