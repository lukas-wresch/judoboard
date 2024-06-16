// Copyright (c) 2004-2011 Sergey Lyubka
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.



#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warning in VS2005
#include "../../ZED/include/log.h"
#if defined(_WIN32)
#pragma comment(lib, "Ws2_32.lib")
#define NO_SSL
#else
#define _XOPEN_SOURCE 600     // For flockfile() on Linux
#define _LARGEFILE_SOURCE     // Enable 64-bit file offsets
#define __STDC_FORMAT_MACROS  // <inttypes.h> wants this for C++
#define __STDC_LIMIT_MACROS   // C++ wants that for INT64_MAX
#endif

#if defined(__SYMBIAN32__)
#define NO_SSL // SSL is not supported
#define NO_CGI // CGI is not supported
#define PATH_MAX FILENAME_MAX
#endif // __SYMBIAN32__

#ifndef _WIN32_WCE // Some ANSI #includes are not available on Windows CE
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#endif // !_WIN32_WCE

#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(__SYMBIAN32__) // Windows specific
#define _WIN32_WINNT 0x0400 // To make it link in VS2005
#include <windows.h>

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#ifndef _WIN32_WCE
#include <process.h>
#include <direct.h>
#include <io.h>
#else // _WIN32_WCE
#include <winsock2.h>
#include <ws2tcpip.h>
#define NO_CGI // WinCE has no pipes

typedef long off_t;

#define errno   GetLastError()
#define strerror(x)  _ultoa(x, (char *) _alloca(sizeof(x) *3 ), 10)
#endif // _WIN32_WCE

#define MAKEUQUAD(lo, hi) ((uint64_t)(((uint32_t)(lo)) | \
      ((uint64_t)((uint32_t)(hi))) << 32))
#define RATE_DIFF 10000000 // 100 nsecs
#define EPOCH_DIFF MAKEUQUAD(0xd53e8000, 0x019db1de)
#define SYS2UNIX_TIME(lo, hi) \
  (time_t) ((MAKEUQUAD((lo), (hi)) - EPOCH_DIFF) / RATE_DIFF)

// Visual Studio 6 does not know __func__ or __FUNCTION__
// The rest of MS compilers use __FUNCTION__, not C99 __func__
// Also use _strtoui64 on modern M$ compilers
#if defined(_MSC_VER) && _MSC_VER < 1300
#define STRX(x) #x
#define STR(x) STRX(x)
#define __func__ "line " STR(__LINE__)
#define strtoull(x, y, z) strtoul(x, y, z)
#define strtoll(x, y, z) strtol(x, y, z)
#else
#define __func__  __FUNCTION__
#define strtoull(x, y, z) _strtoui64(x, y, z)
#define strtoll(x, y, z) _strtoi64(x, y, z)
#endif // _MSC_VER

#define ERRNO   GetLastError()
#define NO_SOCKLEN_T
#define SSL_LIB   "ssleay32.dll"
#define CRYPTO_LIB  "libeay32.dll"
#define DIRSEP '\\'
#define IS_DIRSEP_CHAR(c) ((c) == '/' || (c) == '\\')
#define O_NONBLOCK  0
#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK  WSAEWOULDBLOCK
#endif // !EWOULDBLOCK
#define _POSIX_
#define INT64_FMT  "I64d"

#define WINCDECL __cdecl
#define SHUT_WR 1
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define mg_sleep(x) Sleep(x)

#define pipe(x) _pipe(x, MG_BUF_LEN, _O_BINARY)
#define popen(x, y) _popen(x, y)
#define pclose(x) _pclose(x)
#define close(x) _close(x)
#define dlsym(x,y) GetProcAddress((HINSTANCE) (x), (y))
#define RTLD_LAZY  0
#define fseeko(x, y, z) _lseeki64(_fileno(x), (y), (z))
#define fdopen(x, y) _fdopen((x), (y))
#define write(x, y, z) _write((x), (y), (unsigned) z)
#define read(x, y, z) _read((x), (y), (unsigned) z)
#define flockfile(x) EnterCriticalSection(&global_log_file_lock)
#define funlockfile(x) LeaveCriticalSection(&global_log_file_lock)

#if !defined(fileno)
#define fileno(x) _fileno(x)
#endif // !fileno MINGW #defines fileno


#include "mongoose.h"

static int pthread_mutex_lock(pthread_mutex_t *);
static int pthread_mutex_unlock(pthread_mutex_t *);
static FILE *mg_fopen(const char *path, const char *mode);

#if defined(HAVE_STDINT)
#include <stdint.h>
#else
typedef unsigned int  uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned __int64 uint64_t;
typedef __int64   int64_t;
//#define INT64_MAX  9223372036854775807
#endif // HAVE_STDINT

// POSIX dirent interface
struct dirent {
  char d_name[PATH_MAX];
};

typedef struct DIR {
  HANDLE   handle;
  WIN32_FIND_DATAW info;
  struct dirent  result;
} DIR;

#else    // UNIX  specific
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>

#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#if !defined(NO_SSL_DL) && !defined(NO_SSL)
#include <dlfcn.h>
#endif
#include <pthread.h>
#if defined(__MACH__)
#define SSL_LIB   "libssl.dylib"
#define CRYPTO_LIB  "libcrypto.dylib"
#else
#if !defined(SSL_LIB)
#define SSL_LIB   "libssl.so"
#endif
#if !defined(CRYPTO_LIB)
#define CRYPTO_LIB  "libcrypto.so"
#endif
#endif
#define DIRSEP   '/'
#define IS_DIRSEP_CHAR(c) ((c) == '/')
#ifndef O_BINARY
#define O_BINARY  0
#endif // O_BINARY
#define closesocket(a) close(a)
#define mg_fopen(x, y) fopen(x, y)
#define mg_mkdir(x, y) mkdir(x, y)
#define mg_remove(x) remove(x)
#define mg_rename(x, y) rename(x, y)
#define mg_sleep(x) usleep((x) * 1000)
#define ERRNO errno
#define INVALID_SOCKET (-1)
#define INT64_FMT PRId64
typedef int SOCKET;
#define WINCDECL

#endif // End of Windows and UNIX specific includes

#include "mongoose.h"

#define MONGOOSE_VERSION "3.2"
#define PASSWORDS_FILE_NAME ".htpasswd"
#define CGI_ENVIRONMENT_SIZE 4096
#define MAX_CGI_ENVIR_VARS 64
#define MG_BUF_LEN 8192
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#ifdef _WIN32
static CRITICAL_SECTION global_log_file_lock;
static pthread_t pthread_self(void) {
  return GetCurrentThreadId();
}
#endif // _WIN32

// Darwin prior to 7.0 and Win32 do not have socklen_t
#ifdef NO_SOCKLEN_T
typedef int socklen_t;
#endif // NO_SOCKLEN_T

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

static const char *http_500_error = "Internal Server Error";

#define SSL_ERROR_WANT_READ 2
#define SSL_ERROR_WANT_WRITE 3
#define SSL_FILETYPE_PEM 1
#define CRYPTO_LOCK  1

#if defined(NO_SSL_DL)
#else
// Dynamically loaded SSL functionality
struct ssl_func
{
  const char *name;   // SSL function name
  void  (*ptr)(void); // Function pointer
};

#define SSL_free (* (void (*)(SSL *)) ssl_sw[0].ptr)
#define SSL_accept (* (int (*)(SSL *)) ssl_sw[1].ptr)
#define SSL_connect (* (int (*)(SSL *)) ssl_sw[2].ptr)
#define SSL_read (* (int (*)(SSL *, void *, int)) ssl_sw[3].ptr)
#define SSL_write (* (int (*)(SSL *, const void *,int)) ssl_sw[4].ptr)
#define SSL_get_error (* (int (*)(SSL *, int)) ssl_sw[5].ptr)
#define SSL_set_fd (* (int (*)(SSL *, SOCKET)) ssl_sw[6].ptr)
#define SSL_new (* (SSL * (*)(SSL_CTX *)) ssl_sw[7].ptr)
#define SSL_CTX_new (* (SSL_CTX * (*)(SSL_METHOD *)) ssl_sw[8].ptr)
#define SSLv23_server_method (* (SSL_METHOD * (*)(void)) ssl_sw[9].ptr)
#define SSL_library_init (* (int (*)(void)) ssl_sw[10].ptr)
#define SSL_CTX_use_PrivateKey_file (* (int (*)(SSL_CTX *, \
        const char *, int)) ssl_sw[11].ptr)
#define SSL_CTX_use_certificate_file (* (int (*)(SSL_CTX *, \
        const char *, int)) ssl_sw[12].ptr)
#define SSL_CTX_set_default_passwd_cb \
  (* (void (*)(SSL_CTX *, mg_callback_t)) ssl_sw[13].ptr)
#define SSL_CTX_free (* (void (*)(SSL_CTX *)) ssl_sw[14].ptr)
#define SSL_load_error_strings (* (void (*)(void)) ssl_sw[15].ptr)
#define SSL_CTX_use_certificate_chain_file \
  (* (int (*)(SSL_CTX *, const char *)) ssl_sw[16].ptr)
#define SSLv23_client_method (* (SSL_METHOD * (*)(void)) ssl_sw[17].ptr)

#define CRYPTO_num_locks (* (int (*)(void)) crypto_sw[0].ptr)
#define CRYPTO_set_locking_callback \
  (* (void (*)(void (*)(int, int, const char *, int))) crypto_sw[1].ptr)
#define CRYPTO_set_id_callback \
  (* (void (*)(unsigned long (*)(void))) crypto_sw[2].ptr)
#define ERR_get_error (* (unsigned long (*)(void)) crypto_sw[3].ptr)
#define ERR_error_string (* (char * (*)(unsigned long,char *)) crypto_sw[4].ptr)

// set_ssl_option() function updates this array.
// It loads SSL library dynamically and changes NULLs to the actual addresses
// of respective functions. The macros above (like SSL_connect()) are really
// just calling these functions indirectly via the pointer.
static struct ssl_func ssl_sw[] = {
  {"SSL_free",   NULL},
  {"SSL_accept",   NULL},
  {"SSL_connect",   NULL},
  {"SSL_read",   NULL},
  {"SSL_write",   NULL},
  {"SSL_get_error",  NULL},
  {"SSL_set_fd",   NULL},
  {"SSL_new",   NULL},
  {"SSL_CTX_new",   NULL},
  {"SSLv23_server_method", NULL},
  {"SSL_library_init",  NULL},
  {"SSL_CTX_use_PrivateKey_file", NULL},
  {"SSL_CTX_use_certificate_file",NULL},
  {"SSL_CTX_set_default_passwd_cb",NULL},
  {"SSL_CTX_free",  NULL},
  {"SSL_load_error_strings", NULL},
  {"SSL_CTX_use_certificate_chain_file", NULL},
  {"SSLv23_client_method", NULL},
  {NULL,    NULL}
};

#endif // NO_SSL_DL

static const char *month_names[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


static const char *config_options[] = {
  "C", "cgi_pattern", "**.cgi$|**.pl$|**.php$",
  "E", "cgi_environment", NULL,
  "G", "put_delete_passwords_file", NULL,
  "I", "cgi_interpreter", NULL,
  "P", "protect_uri", NULL,
  "R", "authentication_domain", "mydomain.com",
  "S", "ssi_pattern", "**.shtml$|**.shtm$",
  "a", "access_log_file", NULL,
  "c", "ssl_chain_file", NULL,
  "d", "enable_directory_listing", "yes",
  "e", "error_log_file", NULL,
  "g", "global_passwords_file", NULL,
  "i", "index_files", "index.html,index.htm,index.cgi,index.shtml,index.php",
  "k", "enable_keep_alive", "no",
  "l", "access_control_list", NULL,
  "M", "max_request_size", "16384",
  "m", "extra_mime_types", NULL,
  "p", "listening_ports", "8080",
  "r", "document_root",  ".",
  "s", "ssl_certificate", NULL,
  "t", "num_threads", "10",
  "u", "run_as_user", NULL,
  "w", "url_rewrite_patterns", NULL,
  "x", "hide_files_patterns", NULL,
  NULL
};
#define ENTRIES_PER_CONFIG_OPTION 3

const char **mg_get_valid_option_names(void)
{
  return config_options;
}

static void call_user(struct mg_connection *conn, enum mg_event event)
{
  conn->request_info.user_data = conn->ctx->user_data;
  if (conn->ctx->user_callback)
      conn->ctx->user_callback(event, conn);
}

static int get_option_index(const char *name)
{
  for (int i = 0; config_options[i] != NULL; i += ENTRIES_PER_CONFIG_OPTION)
  {
    if (strcmp(config_options[i], name) == 0 || strcmp(config_options[i + 1], name) == 0)
      return i / ENTRIES_PER_CONFIG_OPTION;
  }
  return -1;
}

const char* mg_get_option(const struct mg_context *ctx, const char *name)
{
  int i;
  if ((i = get_option_index(name)) == -1)
    return NULL;
  else if (ctx->config[i] == NULL)
    return "";
  else
    return ctx->config[i];
}

static void sockaddr_to_string(char *buf, size_t len, const union usa *usa)
{
  buf[0] = '\0';
#if defined(USE_IPV6)
  inet_ntop(usa->sa.sa_family, usa->sa.sa_family == AF_INET ?
            (void *) &usa->sin.sin_addr :
            (void *) &usa->sin6.sin6_addr, buf, len);
#elif defined(_WIN32)
  // Only Windoze Vista (and newer) have inet_ntop()
  strncpy(buf, inet_ntoa(usa->sin.sin_addr), len);
#else
  inet_ntop(usa->sa.sa_family, (void *) &usa->sin.sin_addr, buf, len);
#endif
}

// Print error message to the opened error log stream.
static void cry(struct mg_connection *conn, const char *fmt, ...)
{
  char buf[MG_BUF_LEN], src_addr[20];
  va_list ap;
  FILE *fp;
  time_t timestamp;

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  // Do not lock when getting the callback value, here and below.
  // I suppose this is fine, since function cannot disappear in the
  // same way string option can.
  conn->request_info.log_message = buf;
  //if (call_user(conn, MG_EVENT_LOG) == NULL)
  call_user(conn, MG_EVENT_LOG);
  {
    fp = conn->ctx->config[ERROR_LOG_FILE] == NULL ? NULL :
      mg_fopen(conn->ctx->config[ERROR_LOG_FILE], "a+");

    if (fp)
    {
      flockfile(fp);
      timestamp = time(NULL);

      sockaddr_to_string(src_addr, sizeof(src_addr), &conn->client.rsa);
      fprintf(fp, "[%010lu] [error] [client %s] ", (unsigned long) timestamp, src_addr);

      if (conn->request_info.request_method != NULL)
        fprintf(fp, "%s %s: ", conn->request_info.request_method, conn->request_info.uri);

      fprintf(fp, "%s", buf);
      fputc('\n', fp);
      funlockfile(fp);
      if (fp != stderr)
        fclose(fp);
    }
  }
  conn->request_info.log_message = NULL;
}

// Return fake connection structure. Used for logging, if connection
// is not applicable at the moment of logging.
static struct mg_connection *fc(struct mg_context *ctx)
{
  static struct mg_connection fake_connection;
  fake_connection.ctx = ctx;
  return &fake_connection;
}

const char *mg_version(void)
{
  return MONGOOSE_VERSION;
}

const struct mg_request_info *mg_get_request_info(struct mg_connection *conn) {
  return &conn->request_info;
}

static void mg_strlcpy(char *dst, const char *src, size_t n)
{
  for (; *src != '\0' && n > 1; n--)
    *dst++ = *src++;
  
  *dst = '\0';
}

static int lowercase(const char *s)
{
  return tolower(* (const unsigned char *) s);
}

static int mg_strncasecmp(const char *s1, const char *s2, size_t len)
{
  int diff = 0;

  if (len > 0)
  {
      do
      {
          diff = lowercase(s1++) - lowercase(s2++);
      } while (diff == 0 && s1[-1] != '\0' && --len > 0);
  }

  return diff;
}

static int mg_strcasecmp(const char *s1, const char *s2)
{
  int diff;

  do
  {
    diff = lowercase(s1++) - lowercase(s2++);
  } while (diff == 0 && s1[-1] != '\0');

  return diff;
}

static char * mg_strndup(const char *ptr, size_t len)
{
  char *p;

  if ((p = (char *) malloc(len + 1)) != NULL)
    mg_strlcpy(p, ptr, len + 1);

  return p;
}

static char * mg_strdup(const char *str)
{
  return mg_strndup(str, strlen(str));
}

// Like snprintf(), but never returns negative value, or the value
// that is larger than a supplied buffer.
// Thanks to Adam Zeldis to pointing snprintf()-caused vulnerability
// in his audit report.
static int mg_vsnprintf(struct mg_connection *conn, char *buf, size_t buflen, const char *fmt, va_list ap)
{
  if (buflen == 0)
    return 0;

  int n = vsnprintf(buf, buflen, fmt, ap);

  if (n < 0)
  {
    cry(conn, "vsnprintf error");
    n = 0;
  }
  else if (n >= (int) buflen)
  {
    cry(conn, "truncating vsnprintf buffer: [%.*s]", n > 200 ? 200 : n, buf);
    n = (int) buflen - 1;
  }
  buf[n] = '\0';

  return n;
}

static int mg_snprintf(struct mg_connection *conn, char *buf, size_t buflen, const char *fmt, ...)
{
  va_list ap;
  int n;

  va_start(ap, fmt);
  n = mg_vsnprintf(conn, buf, buflen, fmt, ap);
  va_end(ap);

  return n;
}

// Skip the characters until one of the delimiters characters found.
// 0-terminate resulting word. Skip the delimiter and following whitespaces if any.
// Advance pointer to buffer to the next word. Return found 0-terminated word.
// Delimiters can be quoted with quotechar.
static char *skip_quoted(char **buf, const char *delimiters, const char *whitespace, char quotechar)
{
  char *p, *begin_word, *end_word, *end_whitespace;

  begin_word = *buf;
  end_word = begin_word + strcspn(begin_word, delimiters);

  // Check for quotechar
  if (end_word > begin_word)
  {
    p = end_word - 1;
    while (*p == quotechar)
    {
      // If there is anything beyond end_word, copy it
      if (*end_word == '\0')
      {
        *p = '\0';
        break;
      }
      else
      {
        size_t end_off = strcspn(end_word + 1, delimiters);
        memmove (p, end_word, end_off + 1);
        p += end_off; // p must correspond to end_word - 1
        end_word += end_off + 1;
      }
    }
    for (p++; p < end_word; p++) {
      *p = '\0';
    }
  }

  if (*end_word == '\0') {
    *buf = end_word;
  } else {
    end_whitespace = end_word + 1 + strspn(end_word + 1, whitespace);

    for (p = end_word; p < end_whitespace; p++) {
      *p = '\0';
    }

    *buf = end_whitespace;
  }

  return begin_word;
}

// Simplified version of skip_quoted without quote char
// and whitespace == delimiters
static char *skip(char **buf, const char *delimiters)
{
  return skip_quoted(buf, delimiters, delimiters, 0);
}


// Return HTTP header value, or NULL if not found.
static const char* get_header(const struct mg_request_info *ri, const char *name)
{
  for (int i = 0; i < ri->num_headers; i++)
    if (!mg_strcasecmp(name, ri->http_headers[i].name))
      return ri->http_headers[i].value;

  return NULL;
}

const char* mg_get_header(const struct mg_connection *conn, const char *name)
{
  return get_header(&conn->request_info, name);
}

// A helper function for traversing comma separated list of values.
// It returns a list pointer shifted to the next value, of NULL if the end
// of the list found.
// Value is stored in val vector. If value has form "x=y", then eq_val
// vector is initialized to point to the "y" part, and val vector length
// is adjusted to point only to "x".
static const char *next_option(const char *list, struct vec *val, struct vec *eq_val)
{
  if (list == NULL || *list == '\0')
    list = NULL;// End of the list
  else
  {
    val->ptr = list;
    if ((list = strchr(val->ptr, ',')) != NULL) {
      // Comma found. Store length and shift the list ptr
      val->len = list - val->ptr;
      list++;
    }
    else
    {
      // This value is the last one
      list = val->ptr + strlen(val->ptr);
      val->len = list - val->ptr;
    }

    if (eq_val)
    {
      // Value has form "x=y", adjust pointers and lengths
      // so that val points to "x", and eq_val points to "y".
      eq_val->len = 0;
      eq_val->ptr = (const char *) memchr(val->ptr, '=', val->len);
      if (eq_val->ptr != NULL)
      {
        eq_val->ptr++;  // Skip over '=' character
        eq_val->len = val->ptr + val->len - eq_val->ptr;
        val->len = (eq_val->ptr - val->ptr) - 1;
      }
    }
  }

  return list;
}

static size_t match_prefix(const char *pattern, size_t pattern_len, const char *str)
{
  const char *or_str;
  size_t i, j, len, res;

  if ((or_str = (const char *) memchr(pattern, '|', pattern_len)) != NULL)
  {
    res = match_prefix(pattern, or_str - pattern, str);
    return res > 0 ? res : match_prefix(or_str + 1, (pattern + pattern_len) - (or_str + 1), str);
  }

  i = j = 0;
  res = -1;
  for (; i < pattern_len; i++, j++)
  {
    if (pattern[i] == '?' && str[j] != '\0')
      continue;
    else if (pattern[i] == '$')
      return str[j] == '\0' ? j : -1;
    else if (pattern[i] == '*')
    {
      i++;
      if (pattern[i] == '*')
      {
        i++;
        len = strlen(str + j);
      }
      else
        len = strcspn(str + j, "/");

      if (i == pattern_len)
        return j + len;
      
      do
      {
        res = match_prefix(pattern + i, pattern_len - i, str + j + len);
      } while (res == -1 && len-- > 0);

      return res == -1 ? -1 : j + res + len;
    }
    else if (pattern[i] != str[j])
      return -1;
  }
  return j;
}

// HTTP 1.1 assumes keep alive if "Connection:" header is not set
// This function must tolerate situations when connection info is not
// set up, for example if request parsing failed.
static int should_keep_alive(const struct mg_connection *conn)
{
  const char *http_version = conn->request_info.http_version;
  const char *header = mg_get_header(conn, "Connection");
  if (conn->must_close || conn->request_info.status_code == 401 ||
      mg_strcasecmp(conn->ctx->config[ENABLE_KEEP_ALIVE], "yes") != 0 ||
      (header != NULL && mg_strcasecmp(header, "keep-alive") != 0) ||
      (header == NULL && http_version && strcmp(http_version, "1.1")))
    return 0;
  
  return 1;
}

static const char* suggest_connection_header(const struct mg_connection *conn)
{
  return should_keep_alive(conn) ? "keep-alive" : "close";
}

static void send_http_error(struct mg_connection *conn, int status,
                            const char *reason, const char *fmt, ...) {
  char buf[MG_BUF_LEN];
  va_list ap;
  int len;

  conn->request_info.status_code = status;

  //if (call_user(conn, MG_HTTP_ERROR) == NULL)
  call_user(conn, MG_HTTP_ERROR);
  {
    buf[0] = '\0';
    len = 0;

    // Errors 1xx, 204 and 304 MUST NOT send a body
    if (status > 199 && status != 204 && status != 304) {
      len = mg_snprintf(conn, buf, sizeof(buf), "Error %d: %s", status, reason);
      buf[len++] = '\n';

      va_start(ap, fmt);
      len += mg_vsnprintf(conn, buf + len, sizeof(buf) - len, fmt, ap);
      va_end(ap);
    }
    ZED::Log::Debug(std::string(buf));

    mg_printf(conn, "HTTP/1.1 %d %s\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"
              "Connection: %s\r\n\r\n", status, reason, len,
              suggest_connection_header(conn));
    conn->num_bytes_sent += mg_printf(conn, "%s", buf);
  }
}

#if defined(_WIN32) && !defined(__SYMBIAN32__)
static int pthread_mutex_init(pthread_mutex_t *mutex, void *unused)
{
  unused = NULL;
  *mutex = CreateMutex(NULL, FALSE, NULL);
  return *mutex == NULL ? -1 : 0;
}

static int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  return CloseHandle(*mutex) == 0 ? -1 : 0;
}

static int pthread_mutex_lock(pthread_mutex_t *mutex)
{
  return WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0? 0 : -1;
}

static int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  return ReleaseMutex(*mutex) == 0 ? -1 : 0;
}

static int pthread_cond_init(pthread_cond_t *cv, const void *unused)
{
  unused = NULL;
  cv->signal = CreateEvent(NULL, FALSE, FALSE, NULL);
  cv->broadcast = CreateEvent(NULL, TRUE, FALSE, NULL);
  return cv->signal != NULL && cv->broadcast != NULL ? 0 : -1;
}

static int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex)
{
  HANDLE handles[] = {cv->signal, cv->broadcast};
  ReleaseMutex(*mutex);
  WaitForMultipleObjects(2, handles, FALSE, INFINITE);
  return WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0? 0 : -1;
}

static int pthread_cond_signal(pthread_cond_t *cv)
{
  return SetEvent(cv->signal) == 0 ? -1 : 0;
}

static int pthread_cond_broadcast(pthread_cond_t *cv) {
  // Implementation with PulseEvent() has race condition, see
  // http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
  return PulseEvent(cv->broadcast) == 0 ? -1 : 0;
}

static int pthread_cond_destroy(pthread_cond_t *cv) {
  return CloseHandle(cv->signal) && CloseHandle(cv->broadcast) ? 0 : -1;
}

// For Windows, change all slashes to backslashes in path names.
static void change_slashes_to_backslashes(char *path)
{
  for (int i = 0; path[i] != '\0'; i++)
  {
    if (path[i] == '/')
      path[i] = '\\';
    // i > 0 check is to preserve UNC paths, like \\server\file.txt
    if (path[i] == '\\' && i > 0)
      while (path[i + 1] == '\\' || path[i + 1] == '/')
        memmove(path + i + 1, path + i + 2, strlen(path + i + 1));
  }
}

// Encode 'path' which is assumed UTF-8 string, into UNICODE string.
// wbuf and wbuf_len is a target buffer and its length.
static void to_unicode(const char *path, wchar_t *wbuf, size_t wbuf_len)
{
  char buf[PATH_MAX], buf2[PATH_MAX], *p;

  mg_strlcpy(buf, path, sizeof(buf));
  change_slashes_to_backslashes(buf);

  // Point p to the end of the file name
  p = buf + strlen(buf) - 1;

  // Trim trailing backslash character
  while (p > buf && *p == '\\' && p[-1] != ':')
    *p-- = '\0';
  
   // Protect from CGI code disclosure.
   // This is very nasty hole. Windows happily opens files with
   // some garbage in the end of file name. So fopen("a.cgi    ", "r")
   // actually opens "a.cgi", and does not return an error!
  if (*p == 0x20 ||               // No space at the end
      (*p == 0x2e && p > buf) ||  // No '.' but allow '.' as full path
      *p == 0x2b ||               // No '+'
      (*p & ~0x7f))               // And generally no non-ascii chars
  {
    fprintf(stderr, "Rejecting suspicious path: [%s]", buf);
    wbuf[0] = L'\0';
  }
  else
  {
    // Convert to Unicode and back. If doubly-converted string does not
    // match the original, something is fishy, reject.
    memset(wbuf, 0, wbuf_len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, (int) wbuf_len);
    WideCharToMultiByte(CP_UTF8, 0, wbuf, (int) wbuf_len, buf2, sizeof(buf2),
                        NULL, NULL);
    if (strcmp(buf, buf2) != 0)
      wbuf[0] = L'\0';
  }
}


static FILE *mg_fopen(const char *path, const char *mode) {
  wchar_t wbuf[PATH_MAX], wmode[20];

  to_unicode(path, wbuf, ARRAY_SIZE(wbuf));
  MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, ARRAY_SIZE(wmode));

  return _wfopen(wbuf, wmode);
}

static int mg_stat(const char *path, struct mgstat *stp)
{
  int ok = -1; // Error
  wchar_t wbuf[PATH_MAX];
  WIN32_FILE_ATTRIBUTE_DATA info;

  to_unicode(path, wbuf, ARRAY_SIZE(wbuf));

  if (GetFileAttributesExW(wbuf, GetFileExInfoStandard, &info) != 0)
  {
    stp->size = MAKEUQUAD(info.nFileSizeLow, info.nFileSizeHigh);
    stp->mtime = SYS2UNIX_TIME(info.ftLastWriteTime.dwLowDateTime,
                               info.ftLastWriteTime.dwHighDateTime);
    stp->is_directory =
      info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    ok = 0;  // Success
  }

  return ok;
}

#define set_close_on_exec(fd) // No FD_CLOEXEC on Windows

int mg_start_thread(mg_thread_func_t f, void *p)
{
  return _beginthread((void (__cdecl *)(void *)) f, 0, p) == -1L ? -1 : 0;
}

static HANDLE dlopen(const char *dll_name, int flags)
{
  wchar_t wbuf[PATH_MAX];
  flags = 0; // Unused
  to_unicode(dll_name, wbuf, ARRAY_SIZE(wbuf));
  return LoadLibraryW(wbuf);
}

#if !defined(NO_CGI)
#define SIGKILL 0


#endif // !NO_CGI

static int set_non_blocking_mode(SOCKET sock)
{
  unsigned long on = 1;
  return ioctlsocket(sock, FIONBIO, &on);
}

#else
static int mg_stat(const char *path, struct mgstat *stp) {
  struct stat st;
  int ok;

  if (stat(path, &st) == 0) {
    ok = 0;
    stp->size = st.st_size;
    stp->mtime = st.st_mtime;
    stp->is_directory = S_ISDIR(st.st_mode);
  } else {
    ok = -1;
  }

  return ok;
}

static void set_close_on_exec(int fd) {
  (void) fcntl(fd, F_SETFD, FD_CLOEXEC);
}

int mg_start_thread(mg_thread_func_t func, void *param) {
  pthread_t thread_id;
  pthread_attr_t attr;

  (void) pthread_attr_init(&attr);
  (void) pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  // TODO(lsm): figure out why mongoose dies on Linux if next line is enabled
  // (void) pthread_attr_setstacksize(&attr, sizeof(struct mg_connection) * 5);

  return pthread_create(&thread_id, &attr, func, param);
}

static int set_non_blocking_mode(SOCKET sock)
{
  int flags;

  flags = fcntl(sock, F_GETFL, 0);
  (void) fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  return 0;
}
#endif // _WIN32

// Write data to the IO channel - opened file descriptor, socket or SSL
// descriptor. Return number of bytes written.
static int64_t push(FILE *fp, SOCKET sock, SSL *ssl, const char *buf, int64_t len)
{
  int64_t sent;
  int64_t n, k;

  sent = 0;
  while (sent < len)
  {
    // How many bytes we send in this iteration
    k = len - sent > INT_MAX ? INT_MAX : (int) (len - sent);

    if (ssl)
      n = SSL_write(ssl, buf + sent, (int)k);
    else if (fp)
    {
      n = fwrite(buf + sent, 1, (size_t) k, fp);
      if (ferror(fp))
        n = -1;
    }
    else
      n = send(sock, buf + sent, (size_t) k, MSG_NOSIGNAL);

    if (n < 0)
      break;

    sent += n;
  }

  return sent;
}

// Read from IO channel - opened file descriptor, socket, or SSL descriptor.
// Return number of bytes read.
static int pull(FILE *fp, SOCKET sock, SSL *ssl, char *buf, int len)
{
  int nread;

  if (ssl)
    nread = SSL_read(ssl, buf, len);
  else if (fp)
  {
    // Use read() instead of fread(), because if we're reading from the CGI
    // pipe, fread() may block until IO buffer is filled up. We cannot afford
    // to block and must pass all read bytes immediately to the client.
    nread = read(fileno(fp), buf, (size_t) len);
    if (ferror(fp))
      nread = -1;
  }
  else
    nread = recv(sock, buf, (size_t) len, 0);

  return nread;
}

int mg_read(struct mg_connection *conn, void *buf, size_t len)
{
  int n, buffered_len, nread;
  const char *buffered;

  assert((conn->content_len == -1 && conn->consumed_content == 0) || conn->consumed_content <= conn->content_len);
  //DEBUG_TRACE(("%p %zu %lld %lld", buf, len, conn->content_len, conn->consumed_content));
  nread = 0;
  if (conn->consumed_content < conn->content_len) {

    // Adjust number of bytes to read.
    int64_t to_read = conn->content_len - conn->consumed_content;
    if (to_read < (int64_t) len)
      len = (int) to_read;    

    // How many bytes of data we have buffered in the request buffer?
    buffered = conn->buf + conn->request_len + conn->consumed_content;
    buffered_len = conn->data_len - conn->request_len;
    assert(buffered_len >= 0);

    // Return buffered data back if we haven't done that yet.
    if (conn->consumed_content < (int64_t) buffered_len)
    {
      buffered_len -= (int) conn->consumed_content;
      if (len < (size_t) buffered_len)
        buffered_len = len;
      
      memcpy(buf, buffered, (size_t)buffered_len);
      len -= buffered_len;
      buf = (char *) buf + buffered_len;
      conn->consumed_content += buffered_len;
      nread = buffered_len;
    }

    // We have returned all buffered data. Read new data from the remote socket.
    while (len > 0)
    {
      n = pull(NULL, conn->client.sock, conn->ssl, (char *) buf, (int) len);
      if (n <= 0)
        break;
      
      buf = (char *) buf + n;
      conn->consumed_content += n;
      nread += n;
      len -= n;
    }
  }
  return nread;
}

int64_t mg_write(struct mg_connection *conn, const void *buf, size_t len)
{
  return push(NULL, conn->client.sock, conn->ssl, (const char *) buf, (int64_t) len);
}

int64_t mg_printf(struct mg_connection *conn, const char *fmt, ...)
{
    char buf[MG_BUF_LEN];
    va_list ap;

    va_start(ap, fmt);
    size_t len = mg_vsnprintf(conn, buf, sizeof(buf), fmt, ap);
    va_end(ap);

    return mg_write(conn, buf, len);
}

// URL-decode input buffer into destination buffer.
// 0-terminate the destination buffer. Return the length of decoded data.
// form-url-encoded data differs from URI encoding in a way that it
// uses '+' as character for space, see RFC 1866 section 8.2.1
// http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
static size_t url_decode(const char *src, size_t src_len, char *dst, size_t dst_len, int is_form_url_encoded)
{
  size_t i, j;
  int a, b;
#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')

  for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++)
  {
    if (src[i] == '%' && isxdigit(* (const unsigned char *) (src + i + 1)) && isxdigit(* (const unsigned char *) (src + i + 2)))
    {
      a = tolower(* (const unsigned char *) (src + i + 1));
      b = tolower(* (const unsigned char *) (src + i + 2));
      dst[j] = (char) ((HEXTOI(a) << 4) | HEXTOI(b));
      i += 2;
    }
    else if (is_form_url_encoded && src[i] == '+')
      dst[j] = ' ';
    else
      dst[j] = src[i];
  }

  dst[j] = '\0'; // Null-terminate the destination

  return j;
}

// Scan given buffer and fetch the value of the given variable.
// It can be specified in query string, or in the POST data.
// Return NULL if the variable not found, or allocated 0-terminated value.
// It is caller's responsibility to free the returned value.
int mg_get_var(const char *buf, size_t buf_len, const char *name,
               char *dst, size_t dst_len) {
  const char *p, *e, *s;
  size_t name_len, len;

  name_len = strlen(name);
  e = buf + buf_len;
  len = -1;
  dst[0] = '\0';

  // buf is "var1=val1&var2=val2...". Find variable first
  for (p = buf; p != NULL && p + name_len < e; p++) {
    if ((p == buf || p[-1] == '&') && p[name_len] == '=' && !mg_strncasecmp(name, p, name_len))
    {

      // Point p to variable value
      p += name_len + 1;

      // Point s to the end of the value
      s = (const char *) memchr(p, '&', (size_t)(e - p));
      if (s == NULL)
        s = e;

      assert(s >= p);
      assert( (size_t)(s - p) < dst_len );

      // Decode variable into destination buffer
      if ((size_t) (s - p) < dst_len)
        len = url_decode(p, (size_t)(s - p), dst, dst_len, 1);
      
      break;
    }
  }

  return len;
}



int mg_get_cookie(const struct mg_connection *conn, const char *cookie_name, char *dst, size_t dst_size)
{
  const char *s, *p, *end;
  int name_len, len = -1;

  dst[0] = '\0';
  if ((s = mg_get_header(conn, "Cookie")) == NULL)
    return 0;

  name_len = strlen(cookie_name);
  end = s + strlen(s);

  for (; (s = strstr(s, cookie_name)) != NULL; s += name_len)
    if (s[name_len] == '=') {
      s += name_len + 1;
      if ((p = strchr(s, ' ')) == NULL)
        p = end;
      if (p[-1] == ';')
        p--;
      if (*s == '"' && p[-1] == '"' && p > s + 1)
      {
        s++;
        p--;
      }
      if ((size_t) (p - s) < dst_size)
      {
        len = (p - s) + 1;
        mg_strlcpy(dst, s, (size_t)len);
      }
      break;
    }

  return len;
}

static int convert_uri_to_file_name(struct mg_connection *conn, char *buf, size_t buf_len, struct mgstat *st)
{
  struct vec a, b;
  const char *rewrite, *uri = conn->request_info.uri;
  char *p;
  int match_len, stat_result;

  buf_len--;  // This is because memmove() for PATH_INFO may shift part
              // of the path one byte on the right.
  mg_snprintf(conn, buf, buf_len, "%s%s", conn->ctx->config[DOCUMENT_ROOT],
              uri);

  rewrite = conn->ctx->config[REWRITE];
  while ((rewrite = next_option(rewrite, &a, &b)) != NULL) {
    if ((match_len = match_prefix(a.ptr, a.len, uri)) > 0) {
      mg_snprintf(conn, buf, buf_len, "%.*s%s", b.len, b.ptr, uri + match_len);
      break;
    }
  }

  if ((stat_result = mg_stat(buf, st)) != 0)
  {
    // Support PATH_INFO for CGI scripts.
    for (p = buf + strlen(buf); p > buf + 1; p--)
    {
      if (*p == '/')
      {
        *p = '\0';
        if (match_prefix(conn->ctx->config[CGI_EXTENSIONS], strlen(conn->ctx->config[CGI_EXTENSIONS]), buf) > 0 && (stat_result = mg_stat(buf, st)) == 0)
        {
          // Shift PATH_INFO block one character right, e.g.
          //  "/x.cgi/foo/bar\x00" => "/x.cgi\x00/foo/bar\x00"
          // conn->path_info is pointing to the local variable "path" declared
          // in handle_request(), so PATH_INFO not valid after
          // handle_request returns.
          conn->path_info = p + 1;
          memmove(p + 2, p + 1, strlen(p + 1) + 1);  // +1 is for trailing \0
          p[1] = '/';
          break;
        } else {
          *p = '/';
          stat_result = -1;
        }
      }
    }
  }

  return stat_result;
}

static int sslize(struct mg_connection *conn, SSL_CTX *s, int (*func)(SSL *))
{
  return (conn->ssl = SSL_new(s)) != NULL && SSL_set_fd(conn->ssl, conn->client.sock) == 1 && func(conn->ssl) == 1;
}

// Check whether full request is buffered. Return:
//   -1  if request is malformed
//    0  if request is not yet fully buffered
//   >0  actual request length, including last \r\n\r\n
static int get_request_len(const char *buf, int buflen)
{
    const char *s, *e;
    int len = 0;

    //DEBUG_TRACE(("buf: %p, len: %d", buf, buflen));
    for (s = buf, e = s + buflen - 1; len <= 0 && s < e; s++)
    {
        // Control characters are not allowed but >=128 is.
        if (!isprint(*(const unsigned char*)s) && *s != '\r' && *s != '\n' && *(const unsigned char*)s < 128)
            len = -1;
        else if (s[0] == '\n' && s[1] == '\n')
            len = (int)(s - buf) + 2;
        else if (s[0] == '\n' && &s[1] < e && s[1] == '\r' && s[2] == '\n')
            len = (int)(s - buf) + 3;
    }

    return len;
}

// Convert month to the month number. Return -1 on error, or month number
static int get_month_index(const char *s)
{
  for (size_t i = 0; i < ARRAY_SIZE(month_names); i++)
    if (!strcmp(s, month_names[i]))
      return (int) i;

  return -1;
}

// Parse UTC date-time string, and return the corresponding time_t value.
static time_t parse_date_string(const char *datetime) {
  static const unsigned short days_before_month[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
  };
  char month_str[32];
  int second, minute, hour, day, month, year, leap_days, days;
  time_t result = (time_t) 0;

  if (((sscanf(datetime, "%d/%3s/%d %d:%d:%d",
               &day, month_str, &year, &hour, &minute, &second) == 6) ||
       (sscanf(datetime, "%d %3s %d %d:%d:%d",
               &day, month_str, &year, &hour, &minute, &second) == 6) ||
       (sscanf(datetime, "%*3s, %d %3s %d %d:%d:%d",
               &day, month_str, &year, &hour, &minute, &second) == 6) ||
       (sscanf(datetime, "%d-%3s-%d %d:%d:%d",
               &day, month_str, &year, &hour, &minute, &second) == 6)) &&
      year > 1970 &&
      (month = get_month_index(month_str)) != -1) {
    year -= 1970;
    leap_days = year / 4 - year / 100 + year / 400;
    days = year * 365 + days_before_month[month] + (day - 1) + leap_days;
    result = days * 24 * 3600 + hour * 3600 + minute * 60 + second;
  }

  return result;
}

// Protect against directory disclosure attack by removing '..',
// excessive '/' and '\' characters
static void remove_double_dots_and_double_slashes(char *s) {
  char *p = s;

  while (*s != '\0') {
    *p++ = *s++;
    if (IS_DIRSEP_CHAR(s[-1])) {
      // Skip all following slashes and backslashes
      while (IS_DIRSEP_CHAR(s[0])) {
        s++;
      }

      // Skip all double-dots
      while (*s == '.' && s[1] == '.') {
        s += 2;
      }
    }
  }
  *p = '\0';
}

static const struct {
  const char *extension;
  size_t ext_len;
  const char *mime_type;
} builtin_mime_types[] = {
  {".html", 5, "text/html"},
  {".htm", 4, "text/html"},
  {".shtm", 5, "text/html"},
  {".shtml", 6, "text/html"},
  {".css", 4, "text/css"},
  {".js",  3, "application/x-javascript"},
  {".ico", 4, "image/x-icon"},
  {".gif", 4, "image/gif"},
  {".jpg", 4, "image/jpeg"},
  {".jpeg", 5, "image/jpeg"},
  {".png", 4, "image/png"},
  {".svg", 4, "image/svg+xml"},
  {".txt", 4, "text/plain"},
  {".torrent", 8, "application/x-bittorrent"},
  {".wav", 4, "audio/x-wav"},
  {".mp3", 4, "audio/x-mp3"},
  {".mid", 4, "audio/mid"},
  {".m3u", 4, "audio/x-mpegurl"},
  {".ram", 4, "audio/x-pn-realaudio"},
  {".xml", 4, "text/xml"},
  {".json",  5, "text/json"},
  {".xslt", 5, "application/xml"},
  {".ra",  3, "audio/x-pn-realaudio"},
  {".doc", 4, "application/msword"},
  {".exe", 4, "application/octet-stream"},
  {".zip", 4, "application/x-zip-compressed"},
  {".xls", 4, "application/excel"},
  {".tgz", 4, "application/x-tar-gz"},
  {".tar", 4, "application/x-tar"},
  {".gz",  3, "application/x-gunzip"},
  {".arj", 4, "application/x-arj-compressed"},
  {".rar", 4, "application/x-arj-compressed"},
  {".rtf", 4, "application/rtf"},
  {".pdf", 4, "application/pdf"},
  {".swf", 4, "application/x-shockwave-flash"},
  {".mpg", 4, "video/mpeg"},
  {".webm", 5, "video/webm"},
  {".mpeg", 5, "video/mpeg"},
  {".mp4", 4, "video/mp4"},
  {".m4v", 4, "video/x-m4v"},
  {".asf", 4, "video/x-ms-asf"},
  {".avi", 4, "video/x-msvideo"},
  {".bmp", 4, "image/bmp"},
  {NULL,  0, NULL}
};

const char *mg_get_builtin_mime_type(const char *path) {
  const char *ext;
  size_t i, path_len;

  path_len = strlen(path);

  for (i = 0; builtin_mime_types[i].extension != NULL; i++) {
    ext = path + (path_len - builtin_mime_types[i].ext_len);
    if (path_len > builtin_mime_types[i].ext_len &&
        mg_strcasecmp(ext, builtin_mime_types[i].extension) == 0) {
      return builtin_mime_types[i].mime_type;
    }
  }

  return "text/plain";
}

// Look at the "path" extension and figure what mime type it has.
// Store mime type in the vector.
static void get_mime_type(struct mg_context *ctx, const char *path,
                          struct vec *vec) {
  struct vec ext_vec, mime_vec;
  const char *list, *ext;
  size_t path_len;

  path_len = strlen(path);

  // Scan user-defined mime types first, in case user wants to
  // override default mime types.
  list = ctx->config[EXTRA_MIME_TYPES];
  while ((list = next_option(list, &ext_vec, &mime_vec)) != NULL) {
    // ext now points to the path suffix
    ext = path + path_len - ext_vec.len;
    if (mg_strncasecmp(ext, ext_vec.ptr, ext_vec.len) == 0) {
      *vec = mime_vec;
      return;
    }
  }

  vec->ptr = mg_get_builtin_mime_type(path);
  vec->len = strlen(vec->ptr);
}

// Stringify binary data. Output buffer must be twice as big as input,
// because each byte takes 2 bytes in string representation
static void bin2str(char *to, const unsigned char *p, size_t len) {
  static const char *hex = "0123456789abcdef";

  for (; len--; p++) {
    *to++ = hex[p[0] >> 4];
    *to++ = hex[p[0] & 0x0f];
  }
  *to = '\0';
}

struct de
{
  struct mg_connection *conn;
  char *file_name;
  struct mgstat st;
};

static void url_encode(const char *src, char *dst, size_t dst_len)
{
  static const char *dont_escape = "._-$,;~()";
  static const char *hex = "0123456789abcdef";
  const char *end = dst + dst_len - 1;

  for (; *src != '\0' && dst < end; src++, dst++) {
    if (isalnum(*(const unsigned char *) src) ||
        strchr(dont_escape, * (const unsigned char *) src) != NULL) {
      *dst = *src;
    } else if (dst + 2 < end) {
      dst[0] = '%';
      dst[1] = hex[(* (const unsigned char *) src) >> 4];
      dst[2] = hex[(* (const unsigned char *) src) & 0xf];
      dst += 2;
    }
  }

  *dst = '\0';
}

// Send len bytes from the opened file to the client.
static void send_file_data(struct mg_connection *conn, FILE *fp, int64_t len)
{
  char buf[MG_BUF_LEN];
  int64_t to_read, num_read, num_written;

  while (len > 0)
  {
    // Calculate how much to read from the file in the buffer
    to_read = sizeof(buf);
    if (to_read > len)
      to_read = (int) len;

    // Read from file, exit the loop on error
    if ((num_read = fread(buf, 1, (size_t)to_read, fp)) == 0)
      break;

    // Send read bytes to the client, exit the loop on error
    if ((num_written = mg_write(conn, buf, (size_t)num_read)) != num_read)
      break;

    // Both read and were successful, adjust counters
    conn->num_bytes_sent += num_written;
    len -= num_written;
  }
}

static int parse_range_header(const char *header, int64_t *a, int64_t *b)
{
  return sscanf(header, "bytes=%" INT64_FMT "-%" INT64_FMT, a, b);
}

static void gmt_time_string(char *buf, size_t buf_len, time_t *t)
{
  strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime(t));
}

static void construct_etag(char *buf, size_t buf_len, const struct mgstat *stp)
{
  snprintf(buf, buf_len, "\"%lx.%" INT64_FMT "\"", (unsigned long) stp->mtime, stp->size);
}

static void handle_file_request(struct mg_connection *conn, const char *path, struct mgstat *stp)
{
  char date[64], lm[64], etag[64], range[64];
  const char *msg = "OK", *hdr;
  time_t curtime = time(NULL);
  int64_t cl, r1, r2;
  struct vec mime_vec;
  FILE *fp;
  int n;

  get_mime_type(conn->ctx, path, &mime_vec);
  cl = stp->size;
  conn->request_info.status_code = 200;
  range[0] = '\0';

  if ((fp = mg_fopen(path, "rb")) == NULL)
  {
    send_http_error(conn, 500, http_500_error, "fopen(%s): %s", path, strerror(ERRNO));
    return;
  }
  set_close_on_exec(fileno(fp));

  // If Range: header specified, act accordingly
  r1 = r2 = 0;
  hdr = mg_get_header(conn, "Range");
  if (hdr != NULL && (n = parse_range_header(hdr, &r1, &r2)) > 0) {
    conn->request_info.status_code = 206;
    (void) fseeko(fp, r1, SEEK_SET);
    cl = n == 2 ? r2 - r1 + 1: cl - r1;
    (void) mg_snprintf(conn, range, sizeof(range),
        "Content-Range: bytes "
        "%" INT64_FMT "-%"
        INT64_FMT "/%" INT64_FMT "\r\n",
        r1, r1 + cl - 1, stp->size);
    msg = "Partial Content";
  }

  // Prepare Etag, Date, Last-Modified headers. Must be in UTC, according to
  // http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3
  gmt_time_string(date, sizeof(date), &curtime);
  gmt_time_string(lm, sizeof(lm), &stp->mtime);
  construct_etag(etag, sizeof(etag), stp);

  mg_printf(conn,
      "HTTP/1.1 %d %s\r\n"
      "Date: %s\r\n"
      "Last-Modified: %s\r\n"
      "Etag: %s\r\n"
      "Content-Type: %.*s\r\n"
      "Content-Length: %" INT64_FMT "\r\n"
      "Connection: %s\r\n"
      "Accept-Ranges: bytes\r\n"
      "%s\r\n",
      conn->request_info.status_code, msg, date, lm, etag, (int) mime_vec.len,
      mime_vec.ptr, cl, suggest_connection_header(conn), range);

  if (strcmp(conn->request_info.request_method, "HEAD") != 0)
    send_file_data(conn, fp, cl);
  
  fclose(fp);
}

void mg_send_file(struct mg_connection *conn, const char *path)
{
  struct mgstat st;
  if (mg_stat(path, &st) == 0)
    handle_file_request(conn, path, &st);
  else
    send_http_error(conn, 404, "Not Found", "%s", "File not found");
}


// Parse HTTP headers from the given buffer, advance buffer to the point
// where parsing stopped.
static void parse_http_headers(char **buf, struct mg_request_info *ri)
{
  for (int i = 0; i < (int) ARRAY_SIZE(ri->http_headers); i++)
  {
    ri->http_headers[i].name = skip_quoted(buf, ":", " ", 0);
    ri->http_headers[i].value = skip(buf, "\r\n");
    if (ri->http_headers[i].name[0] == '\0')
      break;
    ri->num_headers = i + 1;
  }
}

static int is_valid_http_method(const char *method)
{
  return !strcmp(method, "GET") || !strcmp(method, "POST") ||
    !strcmp(method, "HEAD") || !strcmp(method, "CONNECT") ||
    !strcmp(method, "PUT") || !strcmp(method, "DELETE") ||
    !strcmp(method, "OPTIONS") || !strcmp(method, "PROPFIND");
}

// Parse HTTP request, fill in mg_request_info structure.
// This function modifies the buffer with HTTP request by nul-terminating
// HTTP request components, header names and header values.
static int parse_http_message(char *buf, int len, struct mg_request_info *ri)
{
  int request_length = get_request_len(buf, len);
  if (request_length > 0)
  {
    // Reset attributes. DO NOT TOUCH is_ssl, remote_ip, remote_port
    ri->remote_user = ri->request_method = ri->uri = ri->http_version = NULL;
    ri->num_headers = 0;
    ri->status_code = -1;

    buf[request_length - 1] = '\0';

    // RFC says that all initial whitespaces should be ingored
    while (*buf != '\0' && isspace(* (unsigned char *) buf)) {
      buf++;
    }
    ri->request_method = skip(&buf, " ");
    ri->uri = skip(&buf, " ");
    ri->http_version = skip(&buf, "\r\n");
    parse_http_headers(&buf, ri);
  }
  return request_length;
}

static int parse_http_request(char *buf, int len, struct mg_request_info *ri) {
  int result = parse_http_message(buf, len, ri);
  if (result > 0 &&
      is_valid_http_method(ri->request_method) &&
      !strncmp(ri->http_version, "HTTP/", 5)) {
    ri->http_version += 5;   // Skip "HTTP/"
  } else {
    result = -1;
  }
  return result;
}

static int parse_http_response(char *buf, int len, struct mg_request_info *ri) {
  int result = parse_http_message(buf, len, ri);
  return result > 0 && !strncmp(ri->request_method, "HTTP/", 5) ? result : -1;
}

// Keep reading the input (either opened file descriptor fd, or socket sock,
// or SSL descriptor ssl) into buffer buf, until \r\n\r\n appears in the
// buffer (which marks the end of HTTP request). Buffer buf may already
// have some data. The length of the data is stored in nread.
// Upon every read operation, increase nread by the number of bytes read.
static int read_request(FILE *fp, SOCKET sock, SSL *ssl, char *buf, int bufsiz,
                        int *nread) {
  int request_len, n = 0;

  do {
    request_len = get_request_len(buf, *nread);
    if (request_len == 0 &&
        (n = pull(fp, sock, ssl, buf + *nread, bufsiz - *nread)) > 0) {
      *nread += n;
    }
  } while (*nread <= bufsiz && request_len == 0 && n > 0);

  return request_len;
}

// For given directory path, substitute it to valid index file.
// Return 0 if index file has been found, -1 if not found.
// If the file is found, it's stats is returned in stp.
static int substitute_index_file(struct mg_connection *conn, char *path,
                                 size_t path_len, struct mgstat *stp) {
  const char *list = conn->ctx->config[INDEX_FILES];
  struct mgstat st;
  struct vec filename_vec;
  size_t n = strlen(path);
  int found = 0;

  // The 'path' given to us points to the directory. Remove all trailing
  // directory separator characters from the end of the path, and
  // then append single directory separator character.
  while (n > 0 && IS_DIRSEP_CHAR(path[n - 1])) {
    n--;
  }
  path[n] = DIRSEP;

  // Traverse index files list. For each entry, append it to the given
  // path and see if the file exists. If it exists, break the loop
  while ((list = next_option(list, &filename_vec, NULL)) != NULL) {

    // Ignore too long entries that may overflow path buffer
    if (filename_vec.len > path_len - (n + 2))
      continue;

    // Prepare full path to the index file
    (void) mg_strlcpy(path + n + 1, filename_vec.ptr, filename_vec.len + 1);

    // Does it exist?
    if (mg_stat(path, &st) == 0) {
      // Yes it does, break the loop
      *stp = st;
      found = 1;
      break;
    }
  }

  // If no index file exists, restore directory path
  if (!found) {
    path[n] = '\0';
  }

  return found;
}

// Return True if we should reply 304 Not Modified.
static int is_not_modified(const struct mg_connection *conn,
                           const struct mgstat *stp) {
  char etag[40];
  const char *ims = mg_get_header(conn, "If-Modified-Since");
  const char *inm = mg_get_header(conn, "If-None-Match");
  construct_etag(etag, sizeof(etag), stp);
  return (inm != NULL && !mg_strcasecmp(etag, inm)) ||
    (ims != NULL && stp->mtime <= parse_date_string(ims));
}

static int forward_body_data(struct mg_connection *conn, FILE *fp,
                             SOCKET sock, SSL *ssl) {
  const char *expect, *buffered;
  char buf[MG_BUF_LEN];
  int to_read, nread, buffered_len, success = 0;

  expect = mg_get_header(conn, "Expect");
  assert(fp != NULL);

  if (conn->content_len == -1) {
    send_http_error(conn, 411, "Length Required", "");
  } else if (expect != NULL && mg_strcasecmp(expect, "100-continue")) {
    send_http_error(conn, 417, "Expectation Failed", "");
  } else {
    if (expect != NULL) {
      (void) mg_printf(conn, "%s", "HTTP/1.1 100 Continue\r\n\r\n");
    }

    buffered = conn->buf + conn->request_len;
    buffered_len = conn->data_len - conn->request_len;
    assert(buffered_len >= 0);
    assert(conn->consumed_content == 0);

    if (buffered_len > 0) {
      if ((int64_t) buffered_len > conn->content_len) {
        buffered_len = (int) conn->content_len;
      }
      push(fp, sock, ssl, buffered, (int64_t) buffered_len);
      conn->consumed_content += buffered_len;
    }

    while (conn->consumed_content < conn->content_len) {
      to_read = sizeof(buf);
      if ((int64_t) to_read > conn->content_len - conn->consumed_content) {
        to_read = (int) (conn->content_len - conn->consumed_content);
      }
      nread = pull(NULL, conn->client.sock, conn->ssl, buf, to_read);
      if (nread <= 0 || push(fp, sock, ssl, buf, nread) != nread) {
        break;
      }
      conn->consumed_content += nread;
    }

    if (conn->consumed_content == conn->content_len) {
      success = 1;
    }

    // Each error code path in this function must send an error
    if (!success) {
      send_http_error(conn, 577, http_500_error, "");
    }
  }

  return success;
}

static void send_options(struct mg_connection *conn)
{
  conn->request_info.status_code = 200;

  mg_printf(conn,
      "HTTP/1.1 200 OK\r\n"
      "Allow: GET, POST, HEAD, CONNECT, PUT, DELETE, OPTIONS\r\n"
      "DAV: 1\r\n\r\n");
}

// Writes PROPFIND properties for a collection element
static void print_props(struct mg_connection *conn, const char* uri,
                        struct mgstat* st) {
  char mtime[64];
  gmt_time_string(mtime, sizeof(mtime), &st->mtime);
  conn->num_bytes_sent += mg_printf(conn,
      "<d:response>"
       "<d:href>%s</d:href>"
       "<d:propstat>"
        "<d:prop>"
         "<d:resourcetype>%s</d:resourcetype>"
         "<d:getcontentlength>%" INT64_FMT "</d:getcontentlength>"
         "<d:getlastmodified>%s</d:getlastmodified>"
        "</d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
       "</d:propstat>"
      "</d:response>\n",
      uri,
      st->is_directory ? "<d:collection/>" : "",
      st->size,
      mtime);
}

static void print_dav_dir_entry(struct de *de, void *data)
{
  char href[PATH_MAX];
  struct mg_connection *conn = (struct mg_connection *) data;
  mg_snprintf(conn, href, sizeof(href), "%s%s", conn->request_info.uri, de->file_name);
  print_props(conn, href, &de->st);
}

static void handle_propfind(struct mg_connection *conn, const char* path, struct mgstat* st)
{
  const char *depth = mg_get_header(conn, "Depth");

  conn->must_close = 1;
  conn->request_info.status_code = 207;
  mg_printf(conn, "HTTP/1.1 207 Multi-Status\r\n"
            "Connection: close\r\n"
            "Content-Type: text/xml; charset=utf-8\r\n\r\n");

  conn->num_bytes_sent += mg_printf(conn,
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
      "<d:multistatus xmlns:d='DAV:'>\n");

  // Print properties for the requested resource itself
  print_props(conn, conn->request_info.uri, st);

  // If it is a directory, print directory entries too if Depth is not 0
  if (st->is_directory &&
      !mg_strcasecmp(conn->ctx->config[ENABLE_DIRECTORY_LISTING], "yes") &&
      (depth == NULL || strcmp(depth, "0") != 0)) {
    //scan_directory(conn, path, conn, &print_dav_dir_entry);
  }

  conn->num_bytes_sent += mg_printf(conn, "%s\n", "</d:multistatus>");
}

// This is the heart of the Mongoose's logic.
// This function is called when the request is read, parsed and validated,
// and Mongoose must decide what action to take: serve a file, or
// a directory, or call embedded function, etcetera.
static void handle_request(struct mg_connection* conn)
{
  struct mg_request_info *ri = &conn->request_info;
  char path[PATH_MAX];
  int stat_result, uri_len;
  struct mgstat st;

  if ((conn->request_info.query_string = strchr(ri->uri, '?')) != NULL)
    *conn->request_info.query_string++ = '\0';

  uri_len = strlen(ri->uri);
  url_decode(ri->uri, (size_t)uri_len, ri->uri, (size_t)(uri_len + 1), 0);
  remove_double_dots_and_double_slashes(ri->uri);
  stat_result = convert_uri_to_file_name(conn, path, sizeof(path), &st);

  ZED::Log::Debug(std::string(ri->uri));
  /*if (!check_authorization(conn, path))
  {
      send_authorization_request(conn);
      //} else if (call_user(conn, MG_NEW_REQUEST) != NULL) {
  }
  else*/
      call_user(conn, MG_NEW_REQUEST);
  /*{
    // Do nothing, callback has served the request
  } else if (!strcmp(ri->request_method, "OPTIONS")) {
    send_options(conn);
  } else if (conn->ctx->config[DOCUMENT_ROOT] == NULL) {
    send_http_error(conn, 404, "Not Found", "Not Found");
  } else if ((!strcmp(ri->request_method, "PUT") ||
        !strcmp(ri->request_method, "DELETE")) &&
      (conn->ctx->config[PUT_DELETE_PASSWORDS_FILE] == NULL ||
       !is_authorized_for_put(conn))) {
    send_authorization_request(conn);
  } else if (!strcmp(ri->request_method, "PUT")) {
    put_file(conn, path);
  } else if (!strcmp(ri->request_method, "DELETE")) {
    if (mg_remove(path) == 0) {
      send_http_error(conn, 200, "OK", "");
    } else {
      send_http_error(conn, 500, http_500_error, "remove(%s): %s", path,
                      strerror(ERRNO));
    }
  } else if (stat_result != 0 || must_hide_file(conn, path)) {
    send_http_error(conn, 404, "Not Found", "%s", "File not found");
  } else if (st.is_directory && ri->uri[uri_len - 1] != '/') {
    (void) mg_printf(conn, "HTTP/1.1 301 Moved Permanently\r\n"
                     "Location: %s/\r\n\r\n", ri->uri);
  } else if (!strcmp(ri->request_method, "PROPFIND")) {
    handle_propfind(conn, path, &st);
  } else if (st.is_directory &&
             !substitute_index_file(conn, path, sizeof(path), &st)) {
    if (!mg_strcasecmp(conn->ctx->config[ENABLE_DIRECTORY_LISTING], "yes")) {
      handle_directory_request(conn, path);
    } else {
      send_http_error(conn, 403, "Directory Listing Denied",
          "Directory listing denied");
    }
#if !defined(NO_CGI)
  } else if (match_prefix(conn->ctx->config[CGI_EXTENSIONS],
                          strlen(conn->ctx->config[CGI_EXTENSIONS]),
                          path) > 0) {
    if (strcmp(ri->request_method, "POST") &&
        strcmp(ri->request_method, "GET")) {
      send_http_error(conn, 501, "Not Implemented",
                      "Method %s is not implemented", ri->request_method);
    } else {
      handle_cgi_request(conn, path);
    }
#endif // !NO_CGI
  } else if (match_prefix(conn->ctx->config[SSI_EXTENSIONS],
                          strlen(conn->ctx->config[SSI_EXTENSIONS]),
                          path) > 0) {
    handle_ssi_file_request(conn, path);
  } else if (is_not_modified(conn, &st)) {
    send_http_error(conn, 304, "Not Modified", "");
  } else {
    handle_file_request(conn, path, &st);
  }*/
}

static void close_all_listening_sockets(struct mg_context *ctx)
{
  struct socket *sp, *tmp;
  for (sp = ctx->listening_sockets; sp != NULL; sp = tmp)
  {
    tmp = sp->next;
    closesocket(sp->sock);
    free(sp);
  }
}

// Valid listening port specification is: [ip_address:]port[s]
// Examples: 80, 443s, 127.0.0.1:3128,1.2.3.4:8080s
// TODO(lsm): add parsing of the IPv6 address
static int parse_port_string(const struct vec *vec, struct socket *so)
{
  int a, b, c, d, port, len;

  // MacOS needs that. If we do not zero it, subsequent bind() will fail.
  // Also, all-zeroes in the socket address means binding to all addresses
  // for both IPv4 and IPv6 (INADDR_ANY and IN6ADDR_ANY_INIT).
  memset(so, 0, sizeof(*so));

  if (sscanf(vec->ptr, "%d.%d.%d.%d:%d%n", &a, &b, &c, &d, &port, &len) == 5)
    so->lsa.sin.sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);// Bind to a specific IPv4 address
  else if (sscanf(vec->ptr, "%d%n", &port, &len) != 1 ||
             len <= 0 || len > (int) vec->len ||
             (vec->ptr[len] && vec->ptr[len] != 's' && vec->ptr[len] != ','))
    return 0;

  so->is_ssl = vec->ptr[len] == 's';
#if defined(USE_IPV6)
  so->lsa.sin6.sin6_family = AF_INET6;
  so->lsa.sin6.sin6_port = htons((uint16_t) port);
#else
  so->lsa.sin.sin_family = AF_INET;
  so->lsa.sin.sin_port = htons((uint16_t) port);
#endif

  return 1;
}



static int set_ports_option(struct mg_context *ctx)
{
  const char *list = ctx->config[LISTENING_PORTS];
#ifdef _WIN32
  char on = 1, success = 1;
#else
  int on = 1, success = 1;
#endif
  SOCKET sock;
  struct vec vec;
  struct socket so, *listener;

  while (success && (list = next_option(list, &vec, NULL)) != NULL)
  {
    if (!parse_port_string(&vec, &so))
    {
      cry(fc(ctx), "%s: %.*s: invalid port spec. Expecting list of: %s", __func__, vec.len, vec.ptr, "[IP_ADDRESS:]PORT[s|p]");
      success = 0;
    }
    else if (so.is_ssl && (ctx->ssl_ctx == NULL || ctx->config[SSL_CERTIFICATE] == NULL))
    {
      cry(fc(ctx), "Cannot add SSL socket, is -ssl_certificate option set?");
      success = 0;
    }
    else if ((sock = socket(so.lsa.sa.sa_family, SOCK_STREAM, 6)) == INVALID_SOCKET ||
//#if !defined(_WIN32)
               // On Windows, SO_REUSEADDR is recommended only for
               // broadcast UDP sockets
               //setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0 ||
//#endif // !_WIN32
               // Set TCP keep-alive. This is needed because if HTTP-level
               // keep-alive is enabled, and client resets the connection,
               // server won't get TCP FIN or RST and will keep the connection
               // open forever. With TCP keep-alive, next keep-alive
               // handshake will figure out that the client is down and
               // will close the server end.
               // Thanks to Igor Klopov who suggested the patch.
               setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) != 0 || bind(sock, &so.lsa.sa, sizeof(so.lsa)) != 0 || listen(sock, 100) != 0)
    {
      closesocket(sock);
      cry(fc(ctx), "%s: cannot bind to %.*s: %s", __func__, vec.len, vec.ptr, strerror(ERRNO));
      success = 0;
    }
    else if ((listener = (struct socket *) calloc(1, sizeof(*listener))) == NULL)
    {
        closesocket(sock);
        cry(fc(ctx), "%s: %s", __func__, strerror(ERRNO));
        success = 0;
    }
    else
    {
        *listener = so;
        listener->sock = sock;
        set_close_on_exec(listener->sock);
        listener->next = ctx->listening_sockets;
        ctx->listening_sockets = listener;
    }
  }

  if (!success)
      close_all_listening_sockets(ctx);

  return success;
}

static int isbyte(int n)
{
  return n >= 0 && n <= 255;
}

// Verify given socket address against the ACL.
// Return -1 if ACL is malformed, 0 if address is disallowed, 1 if allowed.
static int check_acl(struct mg_context *ctx, const union usa *usa)
{
  int a, b, c, d, n, mask, allowed;
  char flag;
  uint32_t acl_subnet, acl_mask, remote_ip;
  struct vec vec;
  const char *list = ctx->config[ACCESS_CONTROL_LIST];

  if (list == NULL)
    return 1;

  memcpy(&remote_ip, &usa->sin.sin_addr, sizeof(remote_ip));

  // If any ACL is set, deny by default
  allowed = '-';

  while ((list = next_option(list, &vec, NULL)) != NULL)
  {
    mask = 32;

    if (sscanf(vec.ptr, "%c%d.%d.%d.%d%n", &flag, &a, &b, &c, &d, &n) != 5)
    {
      cry(fc(ctx), "%s: subnet must be [+|-]x.x.x.x[/x]", __func__);
      return -1;
    }
    else if (flag != '+' && flag != '-') {
      cry(fc(ctx), "%s: flag must be + or -: [%s]", __func__, vec.ptr);
      return -1;
    }
    else if (!isbyte(a)||!isbyte(b)||!isbyte(c)||!isbyte(d))
    {
      cry(fc(ctx), "%s: bad ip address: [%s]", __func__, vec.ptr);
      return -1;
    }
    else if (sscanf(vec.ptr + n, "/%d", &mask) == 0)
    {
      // Do nothing, no mask specified
    }    
    else if (mask < 0 || mask > 32)
    {
      cry(fc(ctx), "%s: bad subnet mask: %d [%s]", __func__, n, vec.ptr);
      return -1;
    }

    acl_subnet = (a << 24) | (b << 16) | (c << 8) | d;
    acl_mask = mask ? 0xffffffffU << (32 - mask) : 0;

    if (acl_subnet == (ntohl(remote_ip) & acl_mask))
      allowed = flag;
  }

  return allowed == '+';
}

static void add_to_set(SOCKET fd, fd_set *set, int *max_fd)
{
  FD_SET(fd, set);
  if (fd > (SOCKET) *max_fd)
    *max_fd = (int) fd;
}

#if !defined(_WIN32)
static int set_uid_option(struct mg_context *ctx) {
  struct passwd *pw;
  const char *uid = ctx->config[RUN_AS_USER];
  int success = 0;

  if (uid == NULL) {
    success = 1;
  } else {
    if ((pw = getpwnam(uid)) == NULL) {
      cry(fc(ctx), "%s: unknown user [%s]", __func__, uid);
    } else if (setgid(pw->pw_gid) == -1) {
      cry(fc(ctx), "%s: setgid(%s): %s", __func__, uid, strerror(errno));
    } else if (setuid(pw->pw_uid) == -1) {
      cry(fc(ctx), "%s: setuid(%s): %s", __func__, uid, strerror(errno));
    } else {
      success = 1;
    }
  }

  return success;
}
#endif // !_WIN32

static int set_gpass_option(struct mg_context *ctx)
{
  struct mgstat mgstat;
  const char *path = ctx->config[GLOBAL_PASSWORDS_FILE];
  return path == NULL || mg_stat(path, &mgstat) == 0;
}

static int set_acl_option(struct mg_context *ctx)
{
  union usa fake;
  return check_acl(ctx, &fake) != -1;
}

static void reset_per_request_attributes(struct mg_connection *conn)
{
  conn->path_info = NULL;
  conn->num_bytes_sent = conn->consumed_content = 0;
  conn->content_len = -1;
  conn->request_len = conn->data_len = 0;
  conn->must_close = 0;
}

static void close_socket_gracefully(SOCKET sock) {
  char buf[MG_BUF_LEN];
  struct linger linger;
  int n;

  // Set linger option to avoid socket hanging out after close. This prevent
  // ephemeral port exhaust problem under high QPS.
  linger.l_onoff = 1;
  linger.l_linger = 1;
  setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

  // Send FIN to the client
  shutdown(sock, SHUT_WR);
  set_non_blocking_mode(sock);

  // Read and discard pending data. If we do not do that and close the
  // socket, the data in the send buffer may be discarded. This
  // behaviour is seen on Windows, when client keeps sending data
  // when server decide to close the connection; then when client
  // does recv() it gets no data back.
  do
  {
    n = pull(NULL, sock, NULL, buf, sizeof(buf));
  } while (n > 0);

  // Now we know that our FIN is ACK-ed, safe to close
  closesocket(sock);
}

static void close_connection(struct mg_connection *conn)
{
  if (conn->ssl)
  {
    SSL_free(conn->ssl);
    conn->ssl = NULL;
  }

  if (conn->client.sock != INVALID_SOCKET)
    close_socket_gracefully(conn->client.sock);
}

void mg_close_connection(struct mg_connection *conn)
{
  close_connection(conn);
  free(conn);
}

struct mg_connection* mg_connect(struct mg_context *ctx, const char *host, int port, int use_ssl)
{
  struct mg_connection *newconn = NULL;
  struct sockaddr_in sin;
  struct hostent *he;
  int sock;

  if (ctx->client_ssl_ctx == NULL && use_ssl)
    cry(fc(ctx), "%s: SSL is not initialized", __func__);
  else if ((he = gethostbyname(host)) == NULL)
    cry(fc(ctx), "%s: gethostbyname(%s): %s", __func__, host, strerror(ERRNO));
  else if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    cry(fc(ctx), "%s: socket: %s", __func__, strerror(ERRNO));
  else
  {
    sin.sin_family = AF_INET;
    sin.sin_port = htons((uint16_t) port);
    sin.sin_addr = * (struct in_addr *) he->h_addr_list[0];
    if (connect(sock, (struct sockaddr *) &sin, sizeof(sin)) != 0)
    {
      cry(fc(ctx), "%s: connect(%s:%d): %s", __func__, host, port, strerror(ERRNO));
      closesocket(sock);
    }
    else if ((newconn = (struct mg_connection *)calloc(1, sizeof(*newconn))) == NULL)
    {
      cry(fc(ctx), "%s: calloc: %s", __func__, strerror(ERRNO));
      closesocket(sock);
    }
    else
    {
      newconn->ctx = ctx;
      newconn->client.sock = sock;
      newconn->client.rsa.sin = sin;
      newconn->client.is_ssl = use_ssl;
      if (use_ssl)
        sslize(newconn, ctx->client_ssl_ctx, SSL_connect);
    }
  }

  return newconn;
}



FILE* mg_fetch(struct mg_context *ctx, const char *url, const char *path, char *buf, size_t buf_len, struct mg_request_info *ri)
{
  struct mg_connection *newconn;
  int n, req_length, data_length, port;
  char host[1025], proto[10], buf2[MG_BUF_LEN];
  FILE *fp = NULL;

  if (sscanf(url, "%9[htps]://%1024[^:]:%d/%n", proto, host, &port, &n) == 3) {
  } else if (sscanf(url, "%9[htps]://%1024[^/]/%n", proto, host, &n) == 2) {
    port = mg_strcasecmp(proto, "https") == 0 ? 443 : 80;
  } else {
    cry(fc(ctx), "%s: invalid URL: [%s]", __func__, url);
    return NULL;
  }

  if ((newconn = mg_connect(ctx, host, port, !strcmp(proto, "https"))) == NULL)
    cry(fc(ctx), "%s: mg_connect(%s): %s", __func__, url, strerror(ERRNO));
  else
  {
    mg_printf(newconn, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", url + n, host);
    data_length = 0;
    req_length = read_request(NULL, newconn->client.sock, newconn->ssl, buf, buf_len, &data_length);

    if (req_length <= 0)
        cry(fc(ctx), "%s(%s): invalid HTTP reply", __func__, url);
    else if (parse_http_response(buf, req_length, ri) <= 0)
        cry(fc(ctx), "%s(%s): cannot parse HTTP headers", __func__, url);
    else if ((fp = fopen(path, "w+b")) == NULL)
        cry(fc(ctx), "%s: fopen(%s): %s", __func__, path, strerror(ERRNO));
    else
    {
      // Write chunk of data that may be in the user's buffer
      data_length -= req_length;
      if (data_length > 0 && fwrite(buf + req_length, 1, data_length, fp) != (size_t) data_length)
      {
        cry(fc(ctx), "%s: fwrite(%s): %s", __func__, path, strerror(ERRNO));
        fclose(fp);
        fp = NULL;
      }
      // Read the rest of the response and write it to the file. Do not use
      // mg_read() cause we didn't set newconn->content_len properly.
      while (fp && (data_length = pull(NULL, newconn->client.sock, newconn->ssl, buf2, sizeof(buf2))) > 0)
      {
        if (fwrite(buf2, 1, data_length, fp) != (size_t) data_length)
        {
          cry(fc(ctx), "%s: fwrite(%s): %s", __func__, path, strerror(ERRNO));
          fclose(fp);
          fp = NULL;
          break;
        }
      }
    }
    mg_close_connection(newconn);
  }

  return fp;
}

static void discard_current_request_from_buffer(struct mg_connection *conn)
{
  char *buffered;
  int buffered_len, body_len;

  buffered = conn->buf + conn->request_len;
  buffered_len = conn->data_len - conn->request_len;
  assert(buffered_len >= 0);

  if (conn->content_len <= 0)
    body_len = 0;// Protect from negative Content-Length, too
  else if (conn->content_len < (int64_t) buffered_len)
    body_len = (int) conn->content_len;
  else
    body_len = buffered_len;

  conn->data_len -= conn->request_len + body_len;
  memmove(conn->buf, conn->buf + conn->request_len + body_len, (size_t) conn->data_len);
}

static int is_valid_uri(const char *uri)
{
  // Conform to http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5.1.2
  // URI can be an asterisk (*) or should start with slash.
  return uri[0] == '/' || (uri[0] == '*' && uri[1] == '\0');
}

static void process_new_connection(struct mg_connection *conn)
{
    struct mg_request_info *ri = &conn->request_info;
    int keep_alive_enabled;
    const char *cl;

    keep_alive_enabled = !strcmp(conn->ctx->config[ENABLE_KEEP_ALIVE], "yes");

    do
    {
        reset_per_request_attributes(conn);
        conn->request_len = read_request(NULL, conn->client.sock, conn->ssl, conn->buf, conn->buf_size, &conn->data_len);
        assert(conn->data_len >= conn->request_len);

        if (conn->request_len == 0 && conn->data_len == conn->buf_size)
        {
            send_http_error(conn, 413, "Request Too Large", "");
            return;
        }
        if (conn->request_len <= 0)
            return;  // Remote end closed the connection

        if (parse_http_request(conn->buf, conn->buf_size, ri) <= 0 || !is_valid_uri(ri->uri))
        {
            // Do not put garbage in the access log, just send it back to the client
            send_http_error(conn, 400, "Bad Request", "Cannot parse HTTP request: [%.*s]", conn->data_len, conn->buf);
        }
        else if (strcmp(ri->http_version, "1.0") && strcmp(ri->http_version, "1.1"))
        {
            // Request seems valid, but HTTP version is strange
            send_http_error(conn, 505, "HTTP version not supported", "");
            //log_access(conn);
        }
        else// Request is valid, handle it
        {
            cl = get_header(ri, "Content-Length");
            conn->content_len = cl == NULL ? -1 : strtoll(cl, NULL, 10);
            conn->birth_time = time(NULL);
            handle_request(conn);
            call_user(conn, MG_REQUEST_COMPLETE);
            //log_access(conn);
            discard_current_request_from_buffer(conn);
        }

        if (ri->remote_user != NULL)
            free((void *) ri->remote_user);
    } while (conn->ctx->stop_flag == 0 && keep_alive_enabled && should_keep_alive(conn));
}

// Worker threads take accepted socket from the queue
static int consume_socket(struct mg_context *ctx, struct socket *sp)
{
  pthread_mutex_lock(&ctx->mutex);
  ctx->free_threads++;
  ZED::Log::Debug(("going idle"));

  // If the queue is empty, wait. We're idle at this point.
  while (ctx->sq_head == ctx->sq_tail && ctx->stop_flag == 0)
    pthread_cond_wait(&ctx->sq_full, &ctx->mutex);

  // If we're stopping, sq_head may be equal to sq_tail.
  if (ctx->sq_head > ctx->sq_tail)
  {
    // Copy socket from the queue and increment tail
    *sp = ctx->queue[ctx->sq_tail % ARRAY_SIZE(ctx->queue)];
    ctx->sq_tail++;
    ctx->free_threads--;
    //ZED::Log::Debug("grabbed socket, going busy " +  std::to_string(sp->sock));

    // Wrap pointers if needed
    while (ctx->sq_tail > (int) ARRAY_SIZE(ctx->queue))
    {
      ctx->sq_tail -= ARRAY_SIZE(ctx->queue);
      ctx->sq_head -= ARRAY_SIZE(ctx->queue);
    }
  }

  pthread_cond_signal(&ctx->sq_empty);
  pthread_mutex_unlock(&ctx->mutex);

  return !ctx->stop_flag;
}

static void worker_thread(struct mg_context *ctx)
{
    int buf_size = atoi(ctx->config[MAX_REQUEST_SIZE]);

    struct mg_connection* conn = (struct mg_connection *) calloc(1, sizeof(*conn) + buf_size);
    if (!conn)
    {
        cry(fc(ctx), "%s", "Cannot create new connection struct, OOM");
        return;
    }

    conn->buf_size = buf_size;
    conn->buf = (char *) (conn + 1);

    // Call consume_socket() even when ctx->stop_flag > 0, to let it signal
    // sq_empty condvar to wake up the master waiting in produce_socket()
    while (consume_socket(ctx, &conn->client))
    {
        conn->birth_time = time(NULL);
        conn->ctx = ctx;

        // Fill in IP, port info early so even if SSL setup below fails,
        // error handler would have the corresponding info.
        // Thanks to Johannes Winkelmann for the patch.
        // TODO(lsm): Fix IPv6 case
        conn->request_info.remote_port = ntohs(conn->client.rsa.sin.sin_port);
        memcpy(&conn->request_info.remote_ip, &conn->client.rsa.sin.sin_addr.s_addr, 4);
        conn->request_info.remote_ip = ntohl(conn->request_info.remote_ip);
        conn->request_info.is_ssl = conn->client.is_ssl;

        if (!conn->client.is_ssl || (conn->client.is_ssl && sslize(conn, conn->ctx->ssl_ctx, SSL_accept)))
            process_new_connection(conn);

        close_connection(conn);
    }

    free(conn);

    // Signal master that we're done with connection and exiting
    pthread_mutex_lock(&ctx->mutex);
    ctx->num_threads--;
    pthread_cond_signal(&ctx->cond);
    assert(ctx->num_threads >= 0);
    pthread_mutex_unlock(&ctx->mutex);

    //ZED::Log::Debug("worker exiting");
}

// Master thread adds accepted socket to a queue
static void produce_socket(struct mg_context *ctx, const struct socket *sp)
{
  pthread_mutex_lock(&ctx->mutex);

  // If the queue is full, wait
  while (ctx->stop_flag == 0 && ctx->sq_head - ctx->sq_tail >= (int) ARRAY_SIZE(ctx->queue))
    pthread_cond_wait(&ctx->sq_empty, &ctx->mutex);

  if (ctx->sq_head - ctx->sq_tail < (int) ARRAY_SIZE(ctx->queue))
  {
    // Copy socket to the queue and increment head
    ctx->queue[ctx->sq_head % ARRAY_SIZE(ctx->queue)] = *sp;
    ctx->sq_head++;
    //ZED::Log::Debug("queued socket " + std::to_string(sp->sock));
  }

  pthread_cond_signal(&ctx->sq_full);
  pthread_mutex_unlock(&ctx->mutex);
}

static void accept_new_connection(const struct socket *listener, struct mg_context *ctx)
{
  struct socket accepted;
  char src_addr[20];

  socklen_t len = sizeof(accepted.rsa);
  accepted.lsa = listener->lsa;
  accepted.sock = accept(listener->sock, &accepted.rsa.sa, &len);
  if (accepted.sock != INVALID_SOCKET)
  {
    int allowed = check_acl(ctx, &accepted.rsa);
    if (allowed)
    {
      //Put accepted socket structure into the queue
      //ZED::Log::Debug("accepted socket " + std::to_string(accepted.sock));
      accepted.is_ssl = listener->is_ssl;
      produce_socket(ctx, &accepted);
    }
    else
    {
      sockaddr_to_string(src_addr, sizeof(src_addr), &accepted.rsa);
      cry(fc(ctx), "%s: %s is not allowed to connect", __func__, src_addr);
      closesocket(accepted.sock);
    }
  }
}

static void master_thread(struct mg_context *ctx)
{
  fd_set read_set;
  struct timeval tv;
  struct socket *sp;
  int max_fd;

  // Increase priority of the master thread
#if defined(_WIN32)
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
  
#if defined(ISSUE_317)
  struct sched_param sched_param;
  sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
  pthread_setschedparam(pthread_self(), SCHED_RR, &sched_param);
#endif

  while (ctx->stop_flag == 0)
  {
    FD_ZERO(&read_set);
    max_fd = -1;

    // Add listening sockets to the read set
    for (sp = ctx->listening_sockets; sp != NULL; sp = sp->next)
      add_to_set(sp->sock, &read_set, &max_fd);

    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;

    if (select(max_fd + 1, &read_set, NULL, NULL, &tv) < 0)
    {
#ifdef _WIN32
      // On windows, if read_set and write_set are empty,
      // select() returns "Invalid parameter" error
      // (at least on my Windows XP Pro). So in this case, we sleep here.
      mg_sleep(1000);
#endif // _WIN32
    }
    else
    {
      for (sp = ctx->listening_sockets; sp != NULL; sp = sp->next)
      {
        if (ctx->stop_flag == 0 && FD_ISSET(sp->sock, &read_set))
          accept_new_connection(sp, ctx);
      }
    }
  }
  ZED::Log::Debug("stopping workers");

  // Stop signal received: somebody called mg_stop. Quit.
  close_all_listening_sockets(ctx);

  // Wakeup workers that are waiting for connections to handle.
  pthread_cond_broadcast(&ctx->sq_full);

  // Wait until all threads finish
  pthread_mutex_lock(&ctx->mutex);
  while (ctx->num_threads > 0)
    pthread_cond_wait(&ctx->cond, &ctx->mutex);
  pthread_mutex_unlock(&ctx->mutex);

  ZED::Log::Debug("all workers stopped");

  // All threads exited, no sync is needed. Destroy mutex and condvars
  pthread_mutex_destroy(&ctx->mutex);
  pthread_cond_destroy(&ctx->cond);
  pthread_cond_destroy(&ctx->sq_empty);
  pthread_cond_destroy(&ctx->sq_full);

  // Signal mg_stop() that we're done
  ctx->stop_flag = 2;

  ZED::Log::Debug("exiting");
}



static void free_context(struct mg_context *ctx)
{
    if (!ctx)
        return;

    // Deallocate config parameters
    for (int i = 0; i < NUM_OPTIONS; i++)
    {
        if (ctx->config[i])
            free(ctx->config[i]);
    }

  // Deallocate SSL context
    if (ctx->ssl_ctx)
        SSL_CTX_free(ctx->ssl_ctx);
  
  if (ctx->client_ssl_ctx)
        SSL_CTX_free(ctx->client_ssl_ctx);

  // Deallocate context itself
  free(ctx);
}



void mg_stop(struct mg_context *ctx)
{
    ctx->stop_flag = 1;

    // Wait until mg_fini() stops
    int i = 0;
    while (ctx->stop_flag != 2)
    {
        if (i >= 10)//After 10 seconds
            break;
        mg_sleep(1000);
        i++;
    }

    mg_sleep(100);
    free_context(ctx);

#if defined(_WIN32) && !defined(__SYMBIAN32__)
    WSACleanup();
#endif // _WIN32
}



struct mg_context* mg_start(void (*user_callback)(enum mg_event event, struct mg_connection* conn), void* user_data, const char** options)
{
    const char *name, *value, *default_value;

#if defined(_WIN32) && !defined(__SYMBIAN32__)
  WSADATA data;
  WSAStartup(MAKEWORD(2,2), &data);
  InitializeCriticalSection(&global_log_file_lock);
#endif // _WIN32

  // Allocate context and initialize reasonable general case defaults.
  // TODO(lsm): do proper error handling here.
  struct mg_context* ctx = (struct mg_context *) calloc(1, sizeof(mg_context));
  ctx->user_callback = user_callback;
  ctx->user_data = user_data;

  int i;
  while (options && (name = *options++) != NULL)
  {
    if ((i = get_option_index(name)) == -1)
    {
      cry(fc(ctx), "Invalid option: %s", name);
      free_context(ctx);
      return NULL;
    }
    else if ((value = *options++) == NULL)
    {
      cry(fc(ctx), "%s: option value cannot be NULL", name);
      free_context(ctx);
      return NULL;
    }
    if (ctx->config[i] != NULL)
      cry(fc(ctx), "warning: %s: duplicate option", name);

    ctx->config[i] = mg_strdup(value);
    ZED::Log::Debug(std::string(name) + " -> " + std::string(value));
  }

  // Set default value if needed
  for (i = 0; config_options[i * ENTRIES_PER_CONFIG_OPTION] != NULL; i++)
  {
    default_value = config_options[i * ENTRIES_PER_CONFIG_OPTION + 2];
    if (ctx->config[i] == NULL && default_value != NULL)
    {
      ctx->config[i] = mg_strdup(default_value);
      ZED::Log::Debug(("Setting default: [%s] -> [%s]", config_options[i * ENTRIES_PER_CONFIG_OPTION + 1], default_value));
    }
  }

  // NOTE(lsm): order is important here. SSL certificates must
  // be initialized before listening ports. UID must be set last.
  //if (!set_gpass_option(ctx) ||
/*#if !defined(NO_SSL)
      !set_ssl_option(ctx) ||
#endif*/
   if (!set_ports_option(ctx) ||
#if !defined(_WIN32)
      !set_uid_option(ctx) ||
#endif
      !set_acl_option(ctx))
   {
       free_context(ctx);
       return NULL;
   }

#if !defined(_WIN32) && !defined(__SYMBIAN32__)
  // Ignore SIGPIPE signal, so if browser cancels the request, it
  // won't kill the whole process.
  signal(SIGPIPE, SIG_IGN);
  // Also ignoring SIGCHLD to let the OS to reap zombies properly.
  signal(SIGCHLD, SIG_IGN);
#endif // !_WIN32

  pthread_mutex_init(&ctx->mutex, NULL);
  pthread_cond_init(&ctx->cond, NULL);
  pthread_cond_init(&ctx->sq_empty, NULL);
  pthread_cond_init(&ctx->sq_full, NULL);

  // Start master (listening) thread
  mg_start_thread((mg_thread_func_t) master_thread, ctx);

  // Start worker threads
  for (i = 0; i < atoi(ctx->config[NUM_THREADS]); i++)
  {
    if (mg_start_thread((mg_thread_func_t) worker_thread, ctx) != 0)
      cry(fc(ctx), "Cannot start worker thread: %d", ERRNO);
    else
      ctx->num_threads++;
  }

  return ctx;
}



int mg_get_free_worker_count(struct mg_context* ctx)
{
    return ctx->free_threads;
}



void mg_increase_worker_count(struct mg_context* ctx, int amount)
{
    // Start worker threads
    for (int i = 0; i < amount; i++)
    {
        if (mg_start_thread((mg_thread_func_t) worker_thread, ctx) != 0)
            cry(fc(ctx), "Cannot start worker thread: %d", ERRNO);
        else
            ctx->num_threads++;
    }
}