#ifndef FILTERING
#define CASE_FIRST(c) case c: {
#define CASE(c) break; } case c: {
#define CASE_FINAL() break; }
#else
#define CASE_FIRST(c) case c: { return true; break; }
#define CASE(c) case c: { return true; break; }
#define CASE_FINAL()
#endif
