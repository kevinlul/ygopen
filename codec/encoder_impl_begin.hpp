#ifndef FILTERING
#define CASE(c, ...) case c: \
	{ \
		__VA_ARGS__ \
		break; \
	}
#else
#define CASE(c, ...) case c: { return true; break; }
#endif // FILTERING
