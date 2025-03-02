# cpplog

## About

Simple, thread-safe, header-only C++ logging library.
This logging library consists of only a single header file and can thus easily be included into any existing project.

## Usage

The logging is handled by the central `Logger` class. This class exposes the three main logging methods `info`, `warn` and `error`, which can be used to log any message or type to the default stream (`stderr`) in a thread-safe manner.
Internally, the `Logger` class uses a `LogImpl` class to handle all of the actual logging.

### Logging of individual types/objects

The `LogImpl` class contains a `log` method that is overloaded for most standard library types and is used to appropriately log thoses types. The following types can be logged by default (assuming that all template parameters of types that take one or more template parameters have an `operator<<` overload):
- all primitive number types
- booleans
- `std::string` or C strings
- `std::pair`
- `std::array` and `std::vector`
- `std::map` and `std::unordered_map`

If you wish to be able to log your own custom types, you can simply extend the `LogImpl` and provide custom overloads for the `log` method for your custom types (see following example):
```
// FILE: MyLogImpl.h

#include "cpplog/logimpl.h"

struct Vec3 {
  float x, y, z;
};

// define overload for operator<< (only required if you want to automatically
// parse log format options with the parse_fmt_opts method)
std::ostream &operator<<(std::ostream &stream, const Vec3 &vec) {
  stream << "vec3: [" << vec.x << ", " << vec.y << ", " << vec.z << "]";
  return stream;
}

/*
 * extends cpplog::LogImpl with custom types
 * so these types can be logged properly as well;
 * if a custom type also overloads operator<<,
 * the parse_fmt_opts method can be used to automatically
 * parse all log format options correctly and to log
 * the custom type based on these options
 */
class MyLogImpl: public cpplog::LogImpl {
 public:
  using cpplog::LogImpl::log;

  inline void log(std::ostream &stream, const Vec3 &vec, cpplog::LogFormat fmt) {
    parse_fmt_opts(stream, vec, fmt);
  }
};
```

### Logging of multiple types/objects

You can also log multiple types by specifying a format string with Python/printf-like syntax. The format string can parse decimal numbers, floating point numbers, strings, objects and characters. Formatting can be specified in curly brackets:
- `"{_>10.2f}"` -> left-padded w/ '_', 10 characters max, floating-point number w/ 2 decimal places
- `"{0>8d< }"`   -> left-padded w/ '0', 8 characters max, decimal number, right-padded w/ ' '

The supported format specifiers are:
- '>' : any character put before '>' will be used to left-pad the to-be-printed type (if a max character size was also specified); '>' should be the 2nd character in the specified format
- max character size: limit the number of characters the to-be-printed output can have; this number should be either the first number of the format or put after the '>' specifier (for left-padding)
- '.' : specify the number of decimal places after a floating-point number after this character
- 'd' | 'f' | 's' | 'o' | 'c' | 'b' : specify that the to-be-printed parameter is a decimal number, floating-point number, string, object, character or boolean
- '<' : any character put after '<' will be used to right-pad the to-be-printed type (if a max character size was also specified); '>' should be the 2nd to last character in the specified format

Please note that all types/objects passed that are supposed to be substituted in the format string must have an `operator<<` overload.

### Code example

Check out the example below to see how this would look inside an actual program.

```
#include "MyLogImpl.h" // see example from above
#include "cpplog/logger.h"

int main() {
  Vec3 vec = Vec3{1, 2, 3};
  cpplog::Logger<MyLogImpl> logger;
  cpplog::LogFormat fmt = cpplog::LogFormatOption::TIMESTAMP | cpplog::LogFormatOption::HIGHLIGHT_GREEN | cpplog::LogFormatOption::TYPE_SIZE;

  logger.info(vec, fmt);  // prints e.g. "[12:30:00] vec3: [1, 2, 3] (SIZE ~= 12 bytes)"

  logger.warn("This is a {#>20s<#}!", "test warning");  // prints "This is a ####test warning####!"

  int p = 10;
  float x = 12.223, y = 145.451, z = 6.1337;
  logger.error("The coordinates of point {0>4d} are: '{ >6.2f}', '{ >6.2f}', '{ >6.2f}'", p, x, y, z);
  // prints "The coordinates of point 0010 are: ' 12.22', '145.45', '  6.13'"
}
```

The `LogFormatOption` enum contains all supported formatting options for a log message/type. You can for example log the current system time at the moment of logging, the (estimated) size of the input type as well as specify the color of the log message. If you also provide an overload for `operator<<` for your custom types, you can simply use the `parse_fmt_opts` method inside your overloaded `log` method to automatically parse the specified log format options and to and to log your custom types based on these options.
