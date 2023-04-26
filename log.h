#ifndef LOG_H_
#define LOG_H_

#include <memory>
#include <mutex>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <ctime>

namespace cpplog {
  // ANSI color codes for colorful logging
  static const char* ANSI_RED     = "\033[31m";
  static const char* ANSI_GREEN   = "\033[32m";
  static const char* ANSI_YELLOW  = "\033[33m";
  static const char* ANSI_DEFAULT = "\033[39m";

  // buffer for current time
  static char time_str[sizeof("hh:mm:ss")];

  // start time of program
  // (will be used to calculate current time whenever a timestamp is logged)
  static std::time_t start_time = std::time(nullptr);

  // available format flags for a log message
  enum LogFmt {
    NEWLINE          = 1 << 0,  // append newline at the end of the log msg
    TIMESTAMP        = 1 << 1,  // add system time at the beginning of log msg
    HIGHLIGHT_RED    = 1 << 2,  // highlight the log msg red
    HIGHLIGHT_GREEN  = 1 << 3,  // highlight the log msg green
    HIGHLIGHT_YELLOW = 1 << 4,  // highlight the log msg yellow
    VERBOSE          = 1 << 5,  // print entire content of type
    TYPE_SIZE        = 1 << 6,  // print (estimated) size of type
    END              = 1 << 7   // end marker
  };

  // available log levels (also compliant with a valid log format)
  enum Level {
    STANDARD = NEWLINE | TIMESTAMP,
    DEBUG    = NEWLINE | TIMESTAMP | TYPE_SIZE
  };

  typedef uint64_t LogFormat;

  /*
   * initialize the log msg based on the log format options;
   * the passed type needs to have an operator<< overload and will be
   * passed directly into the stream, so this function is best only
   * used for primitive types
   */
  template<typename T>
  static void parse_fmt_opts(std::ostream &stream, const T &t, LogFormat fmt) {
    // set color of text
    if (fmt & LogFmt::HIGHLIGHT_GREEN) {
      stream << ANSI_GREEN;
    } else if (fmt & LogFmt::HIGHLIGHT_YELLOW) {
      stream << ANSI_YELLOW;
    } else if (fmt & LogFmt::HIGHLIGHT_RED) {
      stream << ANSI_RED;
    }

    if (fmt & LogFmt::TIMESTAMP) {
      std::strftime(time_str, sizeof(time_str),
                    "%T", std::localtime(&start_time));
      stream << "[" << time_str << "] ";
    }

    // add type to stream (without any special formatting)
    stream << t;

    if (fmt & LogFmt::TYPE_SIZE) {
      stream << " (SIZE = " << sizeof(t) << " bytes)";
    }

    // reset to default colors again
    stream << ANSI_DEFAULT;

    if (fmt & LogFmt::NEWLINE) {
      stream << '\n';
    }
  }

  /*
   * initialize the log msg based on the log format options;
   * this will only set the color of the log msg as well as add
   * the timestamp (if specified)
   */
  static void init_log_msg(std::ostream &stream, LogFormat fmt) {
    // set color of text
    if (fmt & LogFmt::HIGHLIGHT_GREEN) {
      stream << ANSI_GREEN;
    } else if (fmt & LogFmt::HIGHLIGHT_YELLOW) {
      stream << ANSI_YELLOW;
    } else if (fmt & LogFmt::HIGHLIGHT_RED) {
      stream << ANSI_RED;
    }

    if (fmt & LogFmt::TIMESTAMP) {
      std::strftime(time_str, sizeof(time_str),
                    "%T", std::localtime(&start_time));
      stream << "[" << time_str << "] ";
    }
  }

  // log (unsigned) integer types
  static void log(std::ostream &stream, int x, LogFormat fmt) {
    parse_fmt_opts(stream, x, fmt);
  }

  // log floating point types
  static void log(std::ostream &stream, double x, LogFormat fmt) {
    parse_fmt_opts(stream, x, fmt);
  }

/*
 * The Logger class contains a reference to std::cerr, which
 * is the output stream for all logged messages;
 * you can specify the log level and log format of the
 * to-be-logged messages using the set_log_level and
 * set_log_format methods; this class contains methods that
 * support the logging of all primitive and most std library types;
 * if you wish to log your own custom types, please provide
 * a "void log(YourCustomType t)" definition that specifies
 * how that type should be logged; the logging mechanism
 * of this class will then use your custom "log" overload
 * to log this type
 */
class Logger {
 private:
  Level _log_lvl;
  std::ostream &_stream;
  std::string _name;
  LogFormat _log_format;

  // default log formats for error, warning and info messages
  static const LogFormat _default_err_fmt =
    LogFmt::HIGHLIGHT_RED | LogFmt::TIMESTAMP | LogFmt::NEWLINE;
  static const LogFormat _default_warn_fmt =
    LogFmt::HIGHLIGHT_YELLOW | LogFmt::TIMESTAMP | LogFmt::NEWLINE;
  static const LogFormat _default_info_fmt =
    LogFmt::HIGHLIGHT_GREEN | LogFmt::TIMESTAMP | LogFmt::NEWLINE;

 public:
  Logger() :
    _stream(std::cerr) {
    set_log_level(Level::STANDARD);
    set_log_format(Level::STANDARD);
  }

  Logger(const char *name) :
    _stream(std::cerr), _name(name) {
    set_log_level(Level::STANDARD);
    set_log_format(Level::STANDARD);
  }

  Logger(const char *name, Level lvl, LogFormat fmt) :
    _stream(std::cerr), _log_lvl(lvl), _log_format(fmt), _name(name) {
    set_log_level(lvl);
    set_log_format(fmt);
  }

  ~Logger() {}

  // set log level
  void set_log_level(Level lvl) {
    _log_lvl = lvl;
    _log_format = _log_lvl;
  }

  // specify log format (see LogFmt enum for available options)
  void set_log_format(LogFormat fmt) {
    _log_format = fmt;
  }

  template<typename T>
  void error(const T &t, LogFormat fmt) {
    log(_stream, t, fmt | _default_err_fmt);
  }

  template<typename T>
  void error(const T &t) {
    log(_stream, t, _log_format | _default_err_fmt);
  }

  template<typename T>
  void warn(const T &t, LogFormat fmt) {
    log(_stream, t, fmt | _default_warn_fmt);
  }

  template<typename T>
  void warn(const T &t) {
    log(_stream, t, _log_format | _default_warn_fmt);
  }

  template<typename T>
  void info(const T &t, LogFormat fmt) {
    log(_stream, t, fmt | _default_info_fmt);
  }

  template<typename T>
  void info(const T &t) {
    log(_stream, t, _log_format | _default_info_fmt);
  }
};

inline Logger* create_log(const char *name) {
  return new Logger(name);
}

inline Logger* create_log(const char *name, Level lvl, LogFormat fmt) {
  return new Logger(name, lvl, fmt);
}

}  // namespace cpplog

#endif  // LOG_H_
