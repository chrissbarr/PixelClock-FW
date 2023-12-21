#ifndef fmtwrapper_h
#define fmtwrapper_h

#undef B1
#undef F
// #define FMT_HEADER_ONLY
#include <fmt/format.h>
#define F(string_literal) (FPSTR(PSTR(string_literal))) // from WString.h

#include <Print.h>
#include <WString.h>

inline auto format_as(String s) { return s.c_str(); }

#endif // fmtwrapper_h
