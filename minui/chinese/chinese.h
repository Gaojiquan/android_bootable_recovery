
#ifndef CHINESE_H
#define CHINESE_H

// is next character utf-8 encoded cjk character
int ch_test_cjk(const char* s);
// size of the next utf-8 charater
int ch_utf8_length(const char* s);
// convert utf-8 to our custom encoding
int ch_utf8_to_custom(const char* s);
// 2 * wide chars + ascii chars
int str_utf8_length(const char* s);

#endif

