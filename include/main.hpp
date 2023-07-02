#ifndef SHADLP_MAIN_HPP
#define SHADLP_MAIN_HPP

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...)                                                    \
	do {                                                          \
		fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, \
			__LINE__);                                    \
		fprintf(stderr, __VA_ARGS__);                         \
		putc('\n', stderr);                                   \
	} while (0)
#else
#define error(args...)                                                \
	do {                                                          \
		fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, \
			__LINE__);                                    \
		fprintf(stderr, ##args);                              \
		putc('\n', stderr);                                   \
	} while (0)
#endif

#endif // SHADLP_MAIN_HPP

// vim: ts=8 sw=8 noet
