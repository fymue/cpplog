#ifndef LOG_H_
#define LOG_H_

#include <memory>
#include <mutex>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>

namespace cpplog {

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
    STANDARD = NEWLINE | TIMESTAMP | HIGHLIGHT_GREEN,
    DEBUG    = NEWLINE | TIMESTAMP | HIGHLIGHT_GREEN | TYPE_SIZE
  };

  typedef uint64_t LogFormat;

  // initialize the log msg based on the log format options
  // that are indenpendent of the type that is supposed to be logged
  static void parse_fmt_opts(std::ostream &stream, LogFormat fmt) {
    if (fmt & LogFmt::TIMESTAMP) {
    } else if (fmt & LogFmt::HIGHLIGHT_GREEN) {
    } else if (fmt & LogFmt::HIGHLIGHT_YELLOW) {
    } else if (fmt & LogFmt::HIGHLIGHT_RED) {
    }
  }

  // log (unsinged) integer types
  static void log(std::ostream &stream, int x, LogFormat fmt) {
    parse_fmt_opts(stream, fmt);
    stream << x;
    if (fmt & LogFmt::NEWLINE) {
      stream << '\n';
    }
  }

  // log floating point types
  static void log(std::ostream &stream, double x, LogFormat fmt) {
    parse_fmt_opts(stream, fmt);
    stream << x;
    if (fmt & LogFmt::NEWLINE) {
      stream << '\n';
    }
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

  void set_log_level(Level lvl) {
    _log_lvl = lvl;
    switch (_log_lvl) {
      case Level::STANDARD: break;
      case Level::DEBUG: break;
    }
  }

  void set_log_format(LogFormat fmt) {
    _log_format = fmt;
  }

  template<typename T>
  void error(const T &t, LogFormat fmt = _default_err_fmt) {
    log(_stream, t, fmt);
  }

  template<typename T>
  void warn(const T &t, LogFormat fmt = _default_warn_fmt) {
    log(_stream, t, fmt);
  }

  template<typename T>
  void info(const T &t, LogFormat fmt = _default_info_fmt) {
    log(_stream, t, fmt);
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
