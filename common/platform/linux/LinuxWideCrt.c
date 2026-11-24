// =============================================================================
// IrineCore — common/platform/linux/LinuxWideCrt.c
// Pure-C 2-byte UTF-16LE (-fshort-wchar) wide C-runtime overrides for Linux.
// Prevents any translation unit compiled with -fshort-wchar from invoking
// glibc's 4-byte wchar_t implementations, and implements Windows-compatible
// swprintf/vswprintf (%s = wide, %S/%hs = narrow, %I64d/%I64u/%I64x).
// =============================================================================
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

_Static_assert(sizeof(wchar_t) == 2, "LinuxWideCrt.c must be compiled with -fshort-wchar");

// -----------------------------------------------------------------------------
// Wide memory operations
// -----------------------------------------------------------------------------
size_t wcslen(const wchar_t* s)
{
    if (!s) return 0;
    size_t n = 0;
    while (s[n] != 0) ++n;
    return n;
}

size_t wcsnlen(const wchar_t* s, size_t maxlen)
{
    if (!s) return 0;
    size_t n = 0;
    while (n < maxlen && s[n] != 0) ++n;
    return n;
}

wchar_t* wmemcpy(wchar_t* dst, const wchar_t* src, size_t n)
{
    if (n == 0) return dst;
    return (wchar_t*)memcpy(dst, src, n * sizeof(wchar_t));
}

wchar_t* wmemmove(wchar_t* dst, const wchar_t* src, size_t n)
{
    if (n == 0) return dst;
    return (wchar_t*)memmove(dst, src, n * sizeof(wchar_t));
}

wchar_t* wmemset(wchar_t* dst, wchar_t c, size_t n)
{
    for (size_t i = 0; i < n; ++i) dst[i] = c;
    return dst;
}

int wmemcmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        uint16_t a = (uint16_t)s1[i];
        uint16_t b = (uint16_t)s2[i];
        if (a != b) return (a < b) ? -1 : 1;
    }
    return 0;
}

const wchar_t* wmemchr(const wchar_t* s, wchar_t c, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (s[i] == c) return s + i;
    }
    return NULL;
}

// -----------------------------------------------------------------------------
// Wide string operations
// -----------------------------------------------------------------------------
wchar_t* wcscpy(wchar_t* dst, const wchar_t* src)
{
    wchar_t* p = dst;
    while ((*p++ = *src++) != 0) {}
    return dst;
}

wchar_t* wcsncpy(wchar_t* dst, const wchar_t* src, size_t n)
{
    size_t i = 0;
    for (; i < n && src[i] != 0; ++i) dst[i] = src[i];
    for (; i < n; ++i) dst[i] = 0;
    return dst;
}

wchar_t* wcscat(wchar_t* dst, const wchar_t* src)
{
    wchar_t* p = dst;
    while (*p != 0) ++p;
    while ((*p++ = *src++) != 0) {}
    return dst;
}

wchar_t* wcsncat(wchar_t* dst, const wchar_t* src, size_t n)
{
    wchar_t* p = dst;
    while (*p != 0) ++p;
    size_t i = 0;
    for (; i < n && src[i] != 0; ++i) p[i] = src[i];
    p[i] = 0;
    return dst;
}

int wcscmp(const wchar_t* s1, const wchar_t* s2)
{
    while (*s1 && *s1 == *s2) { ++s1; ++s2; }
    uint16_t a = (uint16_t)*s1;
    uint16_t b = (uint16_t)*s2;
    return (a < b) ? -1 : (a > b ? 1 : 0);
}

int wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        uint16_t a = (uint16_t)s1[i];
        uint16_t b = (uint16_t)s2[i];
        if (a != b) return (a < b) ? -1 : 1;
        if (a == 0) return 0;
    }
    return 0;
}

const wchar_t* wcschr(const wchar_t* s, wchar_t c)
{
    for (;; ++s)
    {
        if (*s == c) return s;
        if (*s == 0) return NULL;
    }
}

const wchar_t* wcsrchr(const wchar_t* s, wchar_t c)
{
    const wchar_t* last = NULL;
    for (;; ++s)
    {
        if (*s == c) last = s;
        if (*s == 0) return last;
    }
}

const wchar_t* wcsstr(const wchar_t* haystack, const wchar_t* needle)
{
    if (!needle || needle[0] == 0) return haystack;
    size_t nlen = wcslen(needle);
    for (; *haystack != 0; ++haystack)
    {
        if (*haystack == *needle && wcsncmp(haystack, needle, nlen) == 0)
            return haystack;
    }
    return NULL;
}

const wchar_t* wcspbrk(const wchar_t* s, const wchar_t* accept)
{
    for (; *s != 0; ++s)
    {
        if (wcschr(accept, *s) != NULL) return s;
    }
    return NULL;
}

size_t wcsspn(const wchar_t* s, const wchar_t* accept)
{
    size_t n = 0;
    while (s[n] != 0 && wcschr(accept, s[n]) != NULL) ++n;
    return n;
}

size_t wcscspn(const wchar_t* s, const wchar_t* reject)
{
    size_t n = 0;
    while (s[n] != 0 && wcschr(reject, s[n]) == NULL) ++n;
    return n;
}

wchar_t* wcstok(wchar_t* str, const wchar_t* delim, wchar_t** saveptr)
{
    static __thread wchar_t* tls_save = NULL;
    wchar_t** sp = saveptr ? saveptr : &tls_save;
    wchar_t* s = str ? str : *sp;
    if (!s) return NULL;
    s += wcsspn(s, delim);
    if (*s == 0) { *sp = NULL; return NULL; }
    wchar_t* end = s + wcscspn(s, delim);
    if (*end != 0)
    {
        *end = 0;
        *sp = end + 1;
    }
    else
    {
        *sp = NULL;
    }
    return s;
}

// -----------------------------------------------------------------------------
// Wide character classification (ASCII + basic BMP)
// -----------------------------------------------------------------------------
uint32_t towlower(uint32_t c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

uint32_t towupper(uint32_t c)
{
    return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
}

int iswspace(uint32_t c)  { return (c <= 0x7F) ? isspace((int)c)  : 0; }
int iswdigit(uint32_t c)  { return (c <= 0x7F) ? isdigit((int)c)  : 0; }
int iswalpha(uint32_t c)  { return (c <= 0x7F) ? isalpha((int)c)  : (c >= 0x80 && c <= 0xFFFF); }
int iswalnum(uint32_t c)  { return (c <= 0x7F) ? isalnum((int)c)  : (c >= 0x80 && c <= 0xFFFF); }
int iswupper(uint32_t c)  { return (c <= 0x7F) ? isupper((int)c)  : 0; }
int iswlower(uint32_t c)  { return (c <= 0x7F) ? islower((int)c)  : 0; }
int iswxdigit(uint32_t c) { return (c <= 0x7F) ? isxdigit((int)c) : 0; }
int iswpunct(uint32_t c)  { return (c <= 0x7F) ? ispunct((int)c)  : 0; }
int iswprint(uint32_t c)  { return (c <= 0x7F) ? isprint((int)c)  : (c >= 0x80 && c <= 0xFFFF); }
int iswcntrl(uint32_t c)  { return (c <= 0x7F) ? iscntrl((int)c)  : 0; }

// -----------------------------------------------------------------------------
// Wide numeric conversions
// -----------------------------------------------------------------------------
static size_t w2ascii(const wchar_t* s, char* buf, size_t bufsz)
{
    size_t i = 0;
    if (!s || bufsz == 0) return 0;
    while (s[i] != 0 && i + 1 < bufsz)
    {
        uint16_t ch = (uint16_t)s[i];
        buf[i] = (ch <= 0x7F) ? (char)ch : '?';
        ++i;
    }
    buf[i] = '\0';
    return i;
}

long wcstol(const wchar_t* nptr, wchar_t** endptr, int base)
{
    char buf[256];
    w2ascii(nptr, buf, sizeof(buf));
    char* endc = NULL;
    long v = strtol(buf, &endc, base);
    if (endptr) *endptr = (wchar_t*)(nptr + (endc - buf));
    return v;
}

unsigned long wcstoul(const wchar_t* nptr, wchar_t** endptr, int base)
{
    char buf[256];
    w2ascii(nptr, buf, sizeof(buf));
    char* endc = NULL;
    unsigned long v = strtoul(buf, &endc, base);
    if (endptr) *endptr = (wchar_t*)(nptr + (endc - buf));
    return v;
}

long long wcstoll(const wchar_t* nptr, wchar_t** endptr, int base)
{
    char buf[256];
    w2ascii(nptr, buf, sizeof(buf));
    char* endc = NULL;
    long long v = strtoll(buf, &endc, base);
    if (endptr) *endptr = (wchar_t*)(nptr + (endc - buf));
    return v;
}

unsigned long long wcstoull(const wchar_t* nptr, wchar_t** endptr, int base)
{
    char buf[256];
    w2ascii(nptr, buf, sizeof(buf));
    char* endc = NULL;
    unsigned long long v = strtoull(buf, &endc, base);
    if (endptr) *endptr = (wchar_t*)(nptr + (endc - buf));
    return v;
}

double wcstod(const wchar_t* nptr, wchar_t** endptr)
{
    char buf[256];
    w2ascii(nptr, buf, sizeof(buf));
    char* endc = NULL;
    double v = strtod(buf, &endc);
    if (endptr) *endptr = (wchar_t*)(nptr + (endc - buf));
    return v;
}

float wcstof(const wchar_t* nptr, wchar_t** endptr)
{
    return (float)wcstod(nptr, endptr);
}

// -----------------------------------------------------------------------------
// UTF-16LE <-> UTF-8 internal helpers for printf/stdio
// -----------------------------------------------------------------------------
static void wbuf_push(wchar_t* dst, size_t maxc, size_t* pos, wchar_t ch)
{
    if (dst && maxc > 0 && (*pos + 1) < maxc)
        dst[*pos] = ch;
    (*pos)++;
}

static void wbuf_push_utf8(wchar_t* dst, size_t maxc, size_t* pos, const char* s, int maxchars)
{
    if (!s) s = "(null)";
    size_t i = 0;
    int emitted = 0;
    while (s[i] != '\0' && (maxchars < 0 || emitted < maxchars))
    {
        uint8_t c = (uint8_t)s[i];
        uint32_t cp = 0;
        if (c < 0x80) { cp = c; i += 1; }
        else if ((c & 0xE0) == 0xC0 && s[i + 1])
        {
            cp = ((c & 0x1F) << 6) | ((uint8_t)s[i + 1] & 0x3F);
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0 && s[i + 1] && s[i + 2])
        {
            cp = ((c & 0x0F) << 12) | (((uint8_t)s[i + 1] & 0x3F) << 6) | ((uint8_t)s[i + 2] & 0x3F);
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0 && s[i + 1] && s[i + 2] && s[i + 3])
        {
            cp = ((c & 0x07) << 18) | (((uint8_t)s[i + 1] & 0x3F) << 12) | (((uint8_t)s[i + 2] & 0x3F) << 6) | ((uint8_t)s[i + 3] & 0x3F);
            i += 4;
        }
        else { cp = '?'; i += 1; }

        if (cp <= 0xFFFF)
        {
            wbuf_push(dst, maxc, pos, (wchar_t)cp);
            ++emitted;
        }
        else
        {
            cp -= 0x10000;
            wbuf_push(dst, maxc, pos, (wchar_t)(0xD800 + (cp >> 10)));
            wbuf_push(dst, maxc, pos, (wchar_t)(0xDC00 + (cp & 0x3FF)));
            emitted += 2;
        }
    }
}

// Windows-compatible 2-byte UTF-16LE vswprintf:
//   %s / %ls / %ws -> const wchar_t* (2-byte UTF-16LE)
//   %S / %hs       -> const char*    (UTF-8/ASCII)
//   %c / %lc       -> wchar_t
//   %C / %hc       -> char
//   %I64d / %I64u / %I64x / %lld / %llu / %d / %u / %x / %X / %o / %f / %g / %e / %p
int vswprintf(wchar_t* buffer, size_t maxlen, const wchar_t* format, va_list args)
{
    if (!format)
    {
        if (buffer && maxlen > 0) buffer[0] = 0;
        return 0;
    }

    size_t pos = 0;
    const wchar_t* p = format;
    while (*p != 0)
    {
        if (*p != (wchar_t)'%')
        {
            wbuf_push(buffer, maxlen, &pos, *p++);
            continue;
        }
        ++p;
        if (*p == (wchar_t)'%')
        {
            wbuf_push(buffer, maxlen, &pos, (wchar_t)'%');
            ++p;
            continue;
        }

        // Parse flags
        int left_align = 0;
        int zero_pad = 0;
        int plus_sign = 0;
        int space_sign = 0;
        int alt_form = 0;
        while (*p == (wchar_t)'-' || *p == (wchar_t)'0' || *p == (wchar_t)'+' || *p == (wchar_t)' ' || *p == (wchar_t)'#')
        {
            if (*p == (wchar_t)'-') left_align = 1;
            else if (*p == (wchar_t)'0') zero_pad = 1;
            else if (*p == (wchar_t)'+') plus_sign = 1;
            else if (*p == (wchar_t)' ') space_sign = 1;
            else if (*p == (wchar_t)'#') alt_form = 1;
            ++p;
        }

        // Width
        int width = 0;
        if (*p == (wchar_t)'*')
        {
            width = va_arg(args, int);
            if (width < 0) { left_align = 1; width = -width; }
            ++p;
        }
        else
        {
            while (*p >= (wchar_t)'0' && *p <= (wchar_t)'9')
            {
                width = width * 10 + (*p - (wchar_t)'0');
                ++p;
            }
        }

        // Precision
        int precision = -1;
        if (*p == (wchar_t)'.')
        {
            ++p;
            precision = 0;
            if (*p == (wchar_t)'*')
            {
                precision = va_arg(args, int);
                ++p;
            }
            else
            {
                while (*p >= (wchar_t)'0' && *p <= (wchar_t)'9')
                {
                    precision = precision * 10 + (*p - (wchar_t)'0');
                    ++p;
                }
            }
        }

        // Length modifier
        int len_mod = 0; // 1=h, 2=hh, 3=l, 4=ll/I64, 5=z, 6=w
        if (p[0] == (wchar_t)'I' && p[1] == (wchar_t)'6' && p[2] == (wchar_t)'4')
        {
            len_mod = 4; p += 3;
        }
        else if (p[0] == (wchar_t)'I' && p[1] == (wchar_t)'3' && p[2] == (wchar_t)'2')
        {
            len_mod = 0; p += 3;
        }
        else if (p[0] == (wchar_t)'l' && p[1] == (wchar_t)'l')
        {
            len_mod = 4; p += 2;
        }
        else if (*p == (wchar_t)'l') { len_mod = 3; ++p; }
        else if (p[0] == (wchar_t)'h' && p[1] == (wchar_t)'h') { len_mod = 2; p += 2; }
        else if (*p == (wchar_t)'h') { len_mod = 1; ++p; }
        else if (*p == (wchar_t)'z' || *p == (wchar_t)'t') { len_mod = 5; ++p; }
        else if (*p == (wchar_t)'w') { len_mod = 6; ++p; }
        else if (*p == (wchar_t)'j') { len_mod = 4; ++p; }

        wchar_t spec = *p;
        if (spec == 0) break;
        ++p;

        if ((spec == (wchar_t)'s' && len_mod != 1) || (spec == (wchar_t)'S' && len_mod == 3))
        {
            const wchar_t* ws = va_arg(args, const wchar_t*);
            if (!ws) ws = L"(null)";
            size_t slen = wcslen(ws);
            if (precision >= 0 && (size_t)precision < slen) slen = (size_t)precision;
            int pad = (width > (int)slen) ? (width - (int)slen) : 0;
            if (!left_align) while (pad-- > 0) wbuf_push(buffer, maxlen, &pos, (wchar_t)' ');
            for (size_t i = 0; i < slen; ++i) wbuf_push(buffer, maxlen, &pos, ws[i]);
            if (left_align) while (pad-- > 0) wbuf_push(buffer, maxlen, &pos, (wchar_t)' ');
        }
        else if (spec == (wchar_t)'S' || (spec == (wchar_t)'s' && len_mod == 1))
        {
            const char* ns = va_arg(args, const char*);
            if (!ns) ns = "(null)";
            // Measure UTF-16 length roughly for padding
            size_t start_pos = pos;
            if (width <= 0 || left_align)
            {
                wbuf_push_utf8(buffer, maxlen, &pos, ns, precision);
                size_t written = pos - start_pos;
                int pad = (width > (int)written) ? (width - (int)written) : 0;
                while (pad-- > 0) wbuf_push(buffer, maxlen, &pos, (wchar_t)' ');
            }
            else
            {
                size_t tmp_len = 0;
                wbuf_push_utf8(NULL, 0, &tmp_len, ns, precision);
                int pad = (width > (int)tmp_len) ? (width - (int)tmp_len) : 0;
                while (pad-- > 0) wbuf_push(buffer, maxlen, &pos, (wchar_t)' ');
                wbuf_push_utf8(buffer, maxlen, &pos, ns, precision);
            }
        }
        else if (spec == (wchar_t)'c' || spec == (wchar_t)'C')
        {
            int ch = va_arg(args, int);
            int pad = (width > 1) ? (width - 1) : 0;
            if (!left_align) while (pad-- > 0) wbuf_push(buffer, maxlen, &pos, (wchar_t)' ');
            wbuf_push(buffer, maxlen, &pos, (wchar_t)(ch & 0xFFFF));
            if (left_align) while (pad-- > 0) wbuf_push(buffer, maxlen, &pos, (wchar_t)' ');
        }
        else
        {
            // Delegate numeric / pointer conversion to standard narrow snprintf
            char nfmt[64];
            size_t nf = 0;
            nfmt[nf++] = '%';
            if (left_align) nfmt[nf++] = '-';
            if (plus_sign)  nfmt[nf++] = '+';
            if (space_sign) nfmt[nf++] = ' ';
            if (alt_form)   nfmt[nf++] = '#';
            if (zero_pad)   nfmt[nf++] = '0';
            if (width > 0)  nf += (size_t)snprintf(nfmt + nf, sizeof(nfmt) - nf, "%d", width);
            if (precision >= 0) nf += (size_t)snprintf(nfmt + nf, sizeof(nfmt) - nf, ".%d", precision);

            char tmp[512];
            tmp[0] = '\0';
            char sp = (char)(spec & 0x7F);

            if (sp == 'd' || sp == 'i')
            {
                if (len_mod == 4) { nfmt[nf++] = 'l'; nfmt[nf++] = 'l'; nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, long long)); }
                else if (len_mod == 3) { nfmt[nf++] = 'l'; nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, long)); }
                else if (len_mod == 5) { nfmt[nf++] = 'z'; nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, ptrdiff_t)); }
                else { nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, int)); }
            }
            else if (sp == 'u' || sp == 'x' || sp == 'X' || sp == 'o')
            {
                if (len_mod == 4) { nfmt[nf++] = 'l'; nfmt[nf++] = 'l'; nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, unsigned long long)); }
                else if (len_mod == 3) { nfmt[nf++] = 'l'; nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, unsigned long)); }
                else if (len_mod == 5) { nfmt[nf++] = 'z'; nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, size_t)); }
                else { nfmt[nf++] = sp; nfmt[nf] = 0; snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, unsigned int)); }
            }
            else if (sp == 'f' || sp == 'F' || sp == 'e' || sp == 'E' || sp == 'g' || sp == 'G')
            {
                nfmt[nf++] = sp; nfmt[nf] = 0;
                snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, double));
            }
            else if (sp == 'p')
            {
                nfmt[nf++] = 'p'; nfmt[nf] = 0;
                snprintf(tmp, sizeof(tmp), nfmt, va_arg(args, void*));
            }
            else
            {
                tmp[0] = sp; tmp[1] = '\0';
            }

            for (size_t i = 0; tmp[i] != '\0'; ++i)
                wbuf_push(buffer, maxlen, &pos, (wchar_t)(unsigned char)tmp[i]);
        }
    }

    if (buffer && maxlen > 0)
    {
        size_t nul = (pos < maxlen) ? pos : (maxlen - 1);
        buffer[nul] = 0;
    }
    return (int)pos;
}

int swprintf(wchar_t* buffer, size_t maxlen, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = vswprintf(buffer, maxlen, format, args);
    va_end(args);
    return r;
}

static void fput_utf16_as_utf8(const wchar_t* s, FILE* fp)
{
    if (!s || !fp) return;
    for (size_t i = 0; s[i] != 0; ++i)
    {
        uint32_t cp = (uint16_t)s[i];
        if (cp < 0x80) fputc((int)cp, fp);
        else if (cp < 0x800)
        {
            fputc((int)(0xC0 | (cp >> 6)), fp);
            fputc((int)(0x80 | (cp & 0x3F)), fp);
        }
        else
        {
            fputc((int)(0xE0 | (cp >> 12)), fp);
            fputc((int)(0x80 | ((cp >> 6) & 0x3F)), fp);
            fputc((int)(0x80 | (cp & 0x3F)), fp);
        }
    }
}

int vfwprintf(FILE* stream, const wchar_t* format, va_list args)
{
    wchar_t buf[4096];
    int n = vswprintf(buf, sizeof(buf) / sizeof(buf[0]), format, args);
    fput_utf16_as_utf8(buf, stream);
    return n;
}

int fwprintf(FILE* stream, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vfwprintf(stream, format, args);
    va_end(args);
    return n;
}

int vwprintf(const wchar_t* format, va_list args)
{
    return vfwprintf(stdout, format, args);
}

int wprintf(const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vfwprintf(stdout, format, args);
    va_end(args);
    return n;
}

int fputws(const wchar_t* ws, FILE* stream)
{
    fput_utf16_as_utf8(ws, stream);
    return 0;
}

wchar_t* fgetws(wchar_t* ws, int n, FILE* stream)
{
    if (!ws || n <= 0 || !stream) return NULL;
    char tmp[4096];
    int lim = (n < (int)sizeof(tmp)) ? n : (int)sizeof(tmp);
    if (!fgets(tmp, lim, stream)) return NULL;
    size_t pos = 0;
    wbuf_push_utf8(ws, (size_t)n, &pos, tmp, -1);
    size_t nul = (pos < (size_t)n) ? pos : (size_t)(n - 1);
    ws[nul] = 0;
    return ws;
}

size_t wcsftime(wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr)
{
    char nfmt[256];
    char nout[512];
    w2ascii(format, nfmt, sizeof(nfmt));
    size_t n = strftime(nout, sizeof(nout), nfmt, timeptr);
    if (!ptr || maxsize == 0) return n;
    size_t pos = 0;
    wbuf_push_utf8(ptr, maxsize, &pos, nout, -1);
    size_t nul = (pos < maxsize) ? pos : (maxsize - 1);
    ptr[nul] = 0;
    return nul;
}

int swscanf(const wchar_t* s, const wchar_t* fmt, ...)
{
    // Convert narrow-equivalent ASCII/UTF-8 input & format for standard numeric/token scanning
    char ns[4096];
    char nfmt[512];
    w2ascii(s, ns, sizeof(ns));
    w2ascii(fmt, nfmt, sizeof(nfmt));
    va_list args;
    va_start(args, fmt);
    int r = vsscanf(ns, nfmt, args);
    va_end(args);
    return r;
}
