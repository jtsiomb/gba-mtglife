#ifndef LOGGER_H_
#define LOGGER_H_

enum {
	LOG_SCREEN = 1,
	LOG_DBG = 2
};
#define LOG_ALL		0xffff

void logmsg(unsigned short where, const char *fmt, ...);

#endif	/* LOGGER_H_ */
