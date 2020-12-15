
/* library - fcntl.h */
int creat P((const char *_path, int _mode));
int open P((const char *_path, int _oflag, ...));

/* library - stdlib.h */
double atof P((const char *_str));
void exit P((int _status));

/* library - string.h */
void *memcpy P((void *_t, const void *_s, unsigned _length));
void *memset P((void *_s, int _c, unsigned _nbytes));
char *strcat P((char *_target, const char *_source));
char *strchr P((const char *_s, int _ch));
int strcmp P((const char *_s1, const char *_s2));
char *strcpy P((char *_target, const char *_source));
unsigned strlen P((const char *_s));
char *strncpy P((char *_target, const char *_source, unsigned _maxlength));
char *strrchr P((const char *_s, int _ch));

/* library - unistd.h */
int close P((int _fd));
int isatty P((int _fd));
long lseek P((int _fd, long _offset, int _whence));
int read P((int _fd, char *_buf, unsigned _nbytes));
int write P((int _fd, char *_buf, unsigned _nbytes));

