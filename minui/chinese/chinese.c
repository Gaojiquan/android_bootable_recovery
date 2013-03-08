
#include <string.h>
#include <stdio.h>
#include "chinese.h"

/* Our own notion of wide character, as UCS-4, according to ISO-10646-1. */
typedef unsigned int ucs4_t;
/* Return code if invalid input after a shift sequence of n bytes was read.
   (xxx_mbtowc) */
#define RET_SHIFT_ILSEQ(n)  (-1-2*(n))
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ           RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)       (-2-2*(n))
/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI      -1
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL   -2

static int
utf8_mbtowc (ucs4_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = s[0];

  if (c < 0x80) {
    *pwc = c;
    return 1;
  } else if (c < 0xc2) {
    return RET_ILSEQ;
  } else if (c < 0xe0) {
    if (n < 2)
      return RET_TOOFEW(0);
    if (!((s[1] ^ 0x80) < 0x40))
      return RET_ILSEQ;
    *pwc = ((ucs4_t) (c & 0x1f) << 6)
           | (ucs4_t) (s[1] ^ 0x80);
    return 2;
  } else if (c < 0xf0) {
    if (n < 3)
      return RET_TOOFEW(0);
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (c >= 0xe1 || s[1] >= 0xa0)))
      return RET_ILSEQ;
    *pwc = ((ucs4_t) (c & 0x0f) << 12)
           | ((ucs4_t) (s[1] ^ 0x80) << 6)
           | (ucs4_t) (s[2] ^ 0x80);
    return 3;
  } else if (c < 0xf8 && sizeof(ucs4_t)*8 >= 32) {
    if (n < 4)
      return RET_TOOFEW(0);
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (s[3] ^ 0x80) < 0x40
          && (c >= 0xf1 || s[1] >= 0x90)))
      return RET_ILSEQ;
    *pwc = ((ucs4_t) (c & 0x07) << 18)
           | ((ucs4_t) (s[1] ^ 0x80) << 12)
           | ((ucs4_t) (s[2] ^ 0x80) << 6)
           | (ucs4_t) (s[3] ^ 0x80);
    return 4;
  } else if (c < 0xfc && sizeof(ucs4_t)*8 >= 32) {
    if (n < 5)
      return RET_TOOFEW(0);
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
          && (c >= 0xf9 || s[1] >= 0x88)))
      return RET_ILSEQ;
    *pwc = ((ucs4_t) (c & 0x03) << 24)
           | ((ucs4_t) (s[1] ^ 0x80) << 18)
           | ((ucs4_t) (s[2] ^ 0x80) << 12)
           | ((ucs4_t) (s[3] ^ 0x80) << 6)
           | (ucs4_t) (s[4] ^ 0x80);
    return 5;
  } else if (c < 0xfe && sizeof(ucs4_t)*8 >= 32) {
    if (n < 6)
      return RET_TOOFEW(0);
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
          && (s[5] ^ 0x80) < 0x40
          && (c >= 0xfd || s[1] >= 0x84)))
      return RET_ILSEQ;
    *pwc = ((ucs4_t) (c & 0x01) << 30)
           | ((ucs4_t) (s[1] ^ 0x80) << 24)
           | ((ucs4_t) (s[2] ^ 0x80) << 18)
           | ((ucs4_t) (s[3] ^ 0x80) << 12)
           | ((ucs4_t) (s[4] ^ 0x80) << 6)
           | (ucs4_t) (s[5] ^ 0x80);
    return 6;
  } else
    return RET_ILSEQ;
}

int ch_test_cjk(const char* s)
{
    // fix me
    int ch = (*(const unsigned char*)(s));

    return (ch > 0x80) ? 1 : 0;
}

int ch_utf8_length(const char* s)
{
    int res;
    ucs4_t ch;

    res = utf8_mbtowc(&ch, (const unsigned char*)s, strlen(s));
    if(res <= 0)
        return 0;
    return res;
}

#include "chinese_custom.h"

int ch_utf8_to_custom(const char* s)
{
    int i, n;
    struct utf8_to_custom* res;
    unsigned char name[8];

    n = ch_utf8_length(s);
    if(n <= 0)
        return 0;
    if(n == 1)
        return (*s - 32);
    for(i = 0; i < n; i++)
        name[i] = ((unsigned char*)(s))[i];
    name[n] = 0;
    res = in_word_set(name, n);
    if(res)
        return res->value;
    else
        return 0;
}

int str_utf8_length(const char* s)
{
    int n, l, off;

    n = 0;
    off = 0;
    while(*(s + off))
    {
        l = ch_utf8_length(s + off);
        off += l;
        if(l == 1)
            n += 1;
        else
        {
            // fix me
            n += 2;
        }
    }

    return n;
}

#if 0
#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include "font_10x18.h"
#include "font.h"
static int font_bitmap_count;
static int font_char_per_bitmap = 128;
static void** font_data;
void main()
{
    char* s = "测test试";
    int len;
    unsigned char *bits, *save;
    unsigned char *in, data;
    int sz;
    short ch;
    int i, d, n, bmp, pos;

    sz = 0;
    bits = malloc(font.width * font.height);
    save = bits;
    in = font.rundata;
    while((data = *in++)) {
        memset(bits, (data & 0x80) ? 255 : 0, data & 0x7f);
        bits += (data & 0x7f);
        sz += (data & 0x7f);
    }
    free(save);
    printf("%d * %d %c= %d(%d)\n", font.width, font.height, ((font.width * font.height == sz) ? '=' : '!'), sz, font.width * font.height);
    font_bitmap_count = font.width / font.cwidth / font_char_per_bitmap + 1;
    printf("bitmaps: %d\n", font_bitmap_count);
    font_data = (void**)malloc(font_bitmap_count * sizeof(void*));
    for(n = 0; n < font_bitmap_count; n++)
    {
        font_data[n] = malloc(font_char_per_bitmap * font.cwidth * font.cheight);
        memset(font_data[n], 0, font_char_per_bitmap * font.cwidth * font.cheight);
    }
    d = 0;
    in = font.rundata;
    while((data = *in++))
    {
        n = data & 0x7f;
        for(i = 0; i < n; i++, d++)
        {
            bmp = d % font.width / (font.cwidth * font_char_per_bitmap);
            pos = d / font.width * (font.cwidth * font_char_per_bitmap) + (d % (font.cwidth * font_char_per_bitmap));
            //printf("x = %d y = %d => b = %d x = %d y = %d\n", d % font.width, d / font.width, bmp, pos % (font.cwidth * font_char_per_bitmap), pos / (font.cwidth * font_char_per_bitmap));
            ((unsigned char*)(font_data[bmp]))[pos] = (data & 0x80) ? 0xff : 0;
        }
    }
    printf("%d %d\n", font.width * font.height, d);
    for(n = 0; n < font_bitmap_count; n++)
    {
        free(font_data[n]);
        //free(font_data);
    }
    // 1 0
    printf("%d %d\n", ch_test_cjk(s), ch_test_cjk(s + 3));
    len = ch_utf8_length(s);
    // 3
    printf("%d\n", len);
    len = ch_utf8_length(s+3);
    // 1
    printf("%d\n", len);
    len = str_utf8_length(s);
    // 8
    printf("%s %d\n", s, len);
    len = ch_utf8_to_custom(s);
    // ???
    printf("%d\n", len);
    // 84
    len = ch_utf8_to_custom(s + 3);
    printf("%d\n", len);
}
#endif
