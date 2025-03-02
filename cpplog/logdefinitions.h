#ifndef CPPLOG_LOGDEFINITIONS_H_
#define CPPLOG_LOGDEFINITIONS_H_

#include <cstdint>

namespace cpplog {

// available format flags for a log message
enum LogFormatOption {
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

// defined loggable specifiers for format string of a log message
enum FormatStringSpecifier {
  NONE           = '\0',
  INT            = 'd',
  FLOAT          = 'f',
  HEX            = 'x',
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

// available log levels (also compliant with a valid log format)
enum LogLevel {
  STANDARD = LogFormatOption::NEWLINE | LogFormatOption::TIMESTAMP,
  DEBUG    = LogFormatOption::NEWLINE   | LogFormatOption::TIMESTAMP |
             LogFormatOption::TYPE_SIZE | LogFormatOption::NAME
};

typedef uint64_t LogFormat;

}  // namespace cpplog

#endif  // CPPLOG_LOGDEFINITIONS_H_
