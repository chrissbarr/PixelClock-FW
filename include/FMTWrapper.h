#undef B1
#undef F
//#define FMT_HEADER_ONLY
#include <fmt/format.h>
#define F(string_literal) (FPSTR(PSTR(string_literal))) // from WString.h

#include <WString.h>
#include <Print.h>

inline auto format_as(String s) { return s.c_str(); }

