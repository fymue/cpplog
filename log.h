#ifndef CPPLOG_LOG_H_
#define CPPLOG_LOG_H_

#include <memory>
#include <mutex>
#include <iostream>
#include <fstream>

namespace cpplog {
  enum Level {
    ERROR,
    WARNING,
    INFO,
  };

  class Logger {
   private:
    Level _log_lvl;
    std::ostream &_stream;
    std::string name;

   public:
    Logger() : _log_lvl(Level::ERROR), _stream(std::cerr) {}
  
    Logger(const char *name) : _log_lvl(Level::ERROR),
                               _stream(std::cerr),
                               name(name) {}

    ~Logger() {}

    void set_log_level(Level lvl) {
      _log_lvl = lvl;
      switch (_log_lvl) {
        case Level::ERROR: break;
        case Level::WARNING: break;
        case Level::INFO: break;
      }
    }

    void error(const char *msg) {
      _stream << msg << "\n";
    }

    void warning(const char *msg) {
      _stream << msg << "\n";
    }

    void info(const char *msg) {
      _stream << msg << "\n";
    }
  };

  inline Logger* create_log(const char *name) {
    return new Logger(name);
  }

}

#endif  // CPPLOG_LOG_H_