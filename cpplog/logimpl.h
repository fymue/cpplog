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

#ifndef CPPLOG_LOGIMPL_H_
#define CPPLOG_LOGIMPL_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <utility>
#include <unordered_map>
#include "logdefinitions.h"

namespace cpplog {

/*
 * specifies how a certain datatype should be logged;
 * defines a "inline void log(std::ostream &stream, CustomType t, LogFormat fmt)"
 * method for every loggable type;
 * this class should be extened if you wish to add log definitions
 * for you own custom types as the main "Logger" class will always
 * call a "log" method of whatever LogImpl object it owns
 */
class LogImpl {
  public:
    LogImpl() : start_time(std::time(nullptr)), name("") {}

    ~LogImpl() {}

    LogImpl(const LogImpl &other);

    LogImpl &operator=(const LogImpl &other);

    inline void set_name(const std::string &_name);

    /*
    * initialize the log msg based on the log format options;
    * the passed type needs to have an operator<< overload and
    * will be passed directly into the stream
    */
    template<typename T>
    void parse_fmt_opts(std::ostream &stream, const T &t,
                        LogFormat fmt, size_t type_size);

    template<typename T>
    void parse_fmt_opts(std::ostream &stream, const T &t,
                        LogFormat fmt);

    // log (unsigned) integer types
    void log(std::ostream &stream, int x, LogFormat fmt);

    // log floating point types
    void log(std::ostream &stream, double x, LogFormat fmt);

    // log characters
    void log(std::ostream &stream, char c, LogFormat fmt);

    // log booleans
    void log(std::ostream &stream, bool b, LogFormat fmt);

    // log std::pair
    template<typename T, typename U>
    void log(std::ostream &stream, const std::pair<T, U> &p, LogFormat fmt);

    // log C string
    void log(std::ostream &stream, const char *str, LogFormat fmt);

    // log std::string
    void log(std::ostream &stream, const std::string &str, LogFormat fmt);

    // log std::vector
    template<typename T>
    void log(std::ostream &stream, const std::vector<T> &vec, LogFormat fmt);

    // log std::array
    template<typename T, size_t SIZE>
    void log(std::ostream &stream, const std::array<T, SIZE> &arr,
             LogFormat fmt);

    // log std::map
    template<typename K, typename V>
    void log(std::ostream &stream, const std::map<K, V> &map, LogFormat fmt);

    // log std::unordered_map
    template<typename K, typename V>
    void log(std::ostream &stream, const std::unordered_map<K, V> &map,
             LogFormat fmt);

    template<typename T>
    void log(std::ostream &stream, const T &t, LogFormat fmt);

  private:
    // ANSI color codes for colorful logging
    const char *__ansi_red     = "\033[31m";
    const char *__ansi_green   = "\033[32m";
    const char *__ansi_yellow  = "\033[33m";
    const char *__ansi_default = "\033[39m";

    // limit number of chars of logged string
    const size_t CPPLOG_MX_STR_LEN = 50;

    // limit number of elements of logged container
    const size_t CPPLOG_MX_ELS     = 10;

    // buffer for current time
    char time_str[sizeof("hh:mm:ss")];

    // start time of program
    // (will be used to calculate current time whenever a timestamp is logged)
    std::time_t start_time;

    // name of Logger (for easier differentiation when using multiple Loggers)
    std::string name;
};

inline LogImpl::LogImpl(const LogImpl &other) {
  memcpy(time_str, other.time_str, sizeof("hh:mm:ss"));
  start_time = other.start_time;
  name = other.name;
}

inline LogImpl &LogImpl::operator=(const LogImpl &other) {
  memcpy(time_str, other.time_str, sizeof("hh:mm:ss"));
  start_time = other.start_time;
  name = other.name;
  return *this;
}

inline void LogImpl::set_name(const std::string &_name) {
  name = _name;
}

template<typename T>
inline void LogImpl::parse_fmt_opts(std::ostream &stream, const T &t,
                                    LogFormat fmt, size_t type_size) {
  // set color of text
  if (fmt & LogFormatOption::HIGHLIGHT_GREEN) {
    stream << __ansi_green;
  } else if (fmt & LogFormatOption::HIGHLIGHT_YELLOW) {
    stream << __ansi_yellow;
  } else if (fmt & LogFormatOption::HIGHLIGHT_RED) {
    stream << __ansi_red;
  } else if (fmt & LogFormatOption::HIGHLIGHT_DEF) {
    stream << __ansi_default;
  }

  bool log_name      = fmt & LogFormatOption::NAME;
  bool log_timestamp = fmt & LogFormatOption::TIMESTAMP;

  std::strftime(time_str, sizeof(time_str),
                "%T", std::localtime(&start_time));

  if (log_name && log_timestamp) {
    stream << "[" << name << ", " << time_str << "] ";
  } else {
    if (fmt & LogFormatOption::NAME) {
      stream << "[" << name << "] ";
    }
    if (fmt & LogFormatOption::TIMESTAMP) {
      stream << "[" << time_str << "] ";
    }
  }

  // add type to stream (without any special formatting)
  stream << t;

  // print estimtated size of type (in bytes)
  if (fmt & LogFormatOption::TYPE_SIZE) {
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

  if (fmt & LogFormatOption::NEWLINE) {
    stream << '\n';
  }
}

template<typename T>
inline void LogImpl::parse_fmt_opts(std::ostream &stream, const T &t,
                                    LogFormat fmt) {
  parse_fmt_opts(stream, t, fmt, 0);
}

inline void LogImpl::log(std::ostream &stream, int x, LogFormat fmt) {
  parse_fmt_opts(stream, x, fmt);
}

inline void LogImpl::log(std::ostream &stream, double x, LogFormat fmt) {
  parse_fmt_opts(stream, x, fmt);
}

inline void LogImpl::log(std::ostream &stream, char c, LogFormat fmt) {
  parse_fmt_opts(stream, c, fmt);
}

inline void LogImpl::log(std::ostream &stream, bool b, LogFormat fmt) {
  const char *bool_val = b ? "true" : "false";
  parse_fmt_opts(stream, bool_val, fmt, sizeof(bool));
}

template<typename T, typename U>
inline void LogImpl::log(std::ostream &stream, const std::pair<T, U> &p,
                  LogFormat fmt) {
  parse_fmt_opts(stream, p, fmt);
}

inline void LogImpl::log(std::ostream &stream, const char *str, LogFormat fmt) {
  size_t str_len = std::strlen(str);

  if (fmt & LogFormatOption::NO_SIZE_LIMIT || str_len < CPPLOG_MX_STR_LEN) {
    parse_fmt_opts(stream, str, fmt, str_len);
  } else {
    // if a string is longer than 20 characters,
    // print shortened version of it
    char formatted_str[64];
    int pos = std::snprintf(formatted_str,
                            sizeof(formatted_str), "String: \"");

    size_t mx_border = 8;
    for (size_t i = 0; i < mx_border; ++i, ++pos) {
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

inline void LogImpl::log(std::ostream &stream, const std::string &str,
                         LogFormat fmt) {
  log(stream, str.c_str(), fmt);
}

template<typename T>
inline void LogImpl::log(std::ostream &stream, const std::vector<T> &vec,
                         LogFormat fmt) {
  size_t size = vec.size();
  size_t size_in_bytes = size * sizeof(T);

  if (fmt & LogFormatOption::NO_SIZE_LIMIT || size < CPPLOG_MX_ELS) {
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

template<typename T, size_t SIZE>
inline void LogImpl::log(std::ostream &stream, const std::array<T, SIZE> &arr,
                         LogFormat fmt) {
  size_t size = arr.size();
  size_t size_in_bytes = size * sizeof(T);

  if (fmt & LogFormatOption::NO_SIZE_LIMIT || size < CPPLOG_MX_ELS) {
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

template<typename K, typename V>
inline void LogImpl::log(std::ostream &stream, const std::map<K, V> &map,
                         LogFormat fmt) {
  size_t size = map.size();
  size_t size_in_bytes = size * sizeof(K) + size * sizeof(V);  // estimate

  if (fmt & LogFormatOption::NO_SIZE_LIMIT || size < CPPLOG_MX_ELS) {
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

template<typename K, typename V>
inline void LogImpl::log(std::ostream &stream,
                         const std::unordered_map<K, V> &map, LogFormat fmt) {
  size_t size = map.size();
  size_t size_in_bytes = size * sizeof(K) + size * sizeof(V);  // estimate

  if (fmt & LogFormatOption::NO_SIZE_LIMIT || size < CPPLOG_MX_ELS) {
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

template<typename T>
inline void LogImpl::log(std::ostream &stream, const T &t, LogFormat fmt) {
  std::ostringstream tmp;
  tmp << t;
  parse_fmt_opts(stream, tmp.str(), fmt);
}

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

}  // namespace cpplog

#endif  // CPPLOG_LOGIMPL_H_
