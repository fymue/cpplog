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

#ifndef CPPLOG_LOGDEFINITIONS_H_
#define CPPLOG_LOGDEFINITIONS_H_

#include <cstdint>

namespace cpplog {

typedef uint64_t LogFormat;

// available format flags for a log message
enum LogFormatOption {
  NEWLINE          = 1 << 0,  // append newline at the end of the log msg
  TIMESTAMP        = 1 << 1,  // add system time at the beginning of log msg
  HIGHLIGHT_RED    = 1 << 2,  // highlight the log msg red
  HIGHLIGHT_GREEN  = 1 << 3,  // highlight the log msg green
  HIGHLIGHT_YELLOW = 1 << 4,  // highlight the log msg yellow
  HIGHLIGHT_DEF    = 1 << 5,  // use default terminal color for log msg
  NO_SIZE_LIMIT    = 1 << 6,  // print entire content (for container-like)
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
enum LogVerboseLevel {
  STANDARD = LogFormatOption::NEWLINE | LogFormatOption::TIMESTAMP,
  VERBOSE  = LogFormatOption::NEWLINE   | LogFormatOption::TIMESTAMP |
             LogFormatOption::TYPE_SIZE | LogFormatOption::NAME
};

enum LogOutputLevel {
  QUIET,    // no output at all
  DEFAULT,  // only logging functions info, warn, error work
  DEBUG     // logging function info, warn, error and debug work
};

}  // namespace cpplog

#endif  // CPPLOG_LOGDEFINITIONS_H_
