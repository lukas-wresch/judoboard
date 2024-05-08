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

#ifndef MONGOOSE_HEADER_INCLUDED
#define MONGOOSE_HEADER_INCLUDED

#include <stddef.h>
#include <stdio.h>
#include <atomic>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct mg_context;     // Handle for the HTTP service itself
struct mg_connection;  // Handle for the individual connection


// This structure contains information about the HTTP request.
struct mg_request_info
{
  void *user_data;       // User-defined pointer passed to mg_start()
  char *request_method;  // "GET", "POST", etc
  char *uri;             // URL-decoded URI
  char *http_version;    // E.g. "1.0", "1.1"
  char *query_string;    // URL part after '?' (not including '?') or NULL
  char *remote_user;     // Authenticated user, or NULL if no auth used
  char *log_message;     // Mongoose error log message, MG_EVENT_LOG only
  long remote_ip;        // Client's IP address
  int remote_port;       // Client's port
  int status_code;       // HTTP reply status code, e.g. 200
  int is_ssl;            // 1 if SSL-ed, 0 if not
  int num_headers;       // Number of headers
  struct mg_header {
    char *name;          // HTTP header name
    char *value;         // HTTP header value
  } http_headers[64];    // Maximum 64 headers
};

// Various events on which user-defined function is called by Mongoose.
enum mg_event : uint32_t
{
  MG_NEW_REQUEST,   // New HTTP request has arrived from the client
  MG_HTTP_ERROR,    // HTTP error must be returned to the client
  MG_EVENT_LOG,     // Mongoose logs an event, request_info.log_message
  MG_INIT_SSL,      // Mongoose initializes SSL. Instead of mg_connection *,
                    // SSL context is passed to the callback function.
  MG_REQUEST_COMPLETE  // Mongoose has finished handling the request
};

// Snatched from OpenSSL includes. I put the prototypes here to be independent
// from the OpenSSL source installation. Having this, mongoose + SSL can be
// built on any system with binary SSL libraries installed.
typedef struct ssl_st SSL;
typedef struct ssl_method_st SSL_METHOD;
typedef struct ssl_ctx_st SSL_CTX;


// NOTE(lsm): this enum shoulds be in sync with the config_options below.
enum
{
    CGI_EXTENSIONS, CGI_ENVIRONMENT, PUT_DELETE_PASSWORDS_FILE, CGI_INTERPRETER,
    PROTECT_URI, AUTHENTICATION_DOMAIN, SSI_EXTENSIONS, ACCESS_LOG_FILE,
    SSL_CHAIN_FILE, ENABLE_DIRECTORY_LISTING, ERROR_LOG_FILE,
    GLOBAL_PASSWORDS_FILE, INDEX_FILES,
    ENABLE_KEEP_ALIVE, ACCESS_CONTROL_LIST, MAX_REQUEST_SIZE,
    EXTRA_MIME_TYPES, LISTENING_PORTS, DOCUMENT_ROOT, SSL_CERTIFICATE,
    NUM_THREADS, RUN_AS_USER, REWRITE, HIDE_FILES,
    NUM_OPTIONS
};


// Prototype for the user-defined function. Mongoose calls this function
// on every MG_* event.
//
// Parameters:
//   event: which event has been triggered.
//   conn: opaque connection handler. Could be used to read, write data to the
//         client, etc. See functions below that have "mg_connection *" arg.
//
// Return:
//   If handler returns non-NULL, that means that handler has processed the
//   request by sending appropriate HTTP reply to the client. Mongoose treats
//   the request as served.
//   If handler returns NULL, that means that handler has not processed
//   the request. Handler must not send any data to the client in this case.
//   Mongoose proceeds with request handling as if nothing happened.
typedef void (*mg_callback_t)(enum mg_event event, struct mg_connection* conn);


// Unified socket address. For IPv6 support, add IPv6 address structure
// in the union u.
union usa
{
    struct sockaddr sa;
    struct sockaddr_in sin;
#if defined(USE_IPV6)
    struct sockaddr_in6 sin6;
#endif
};

// Describes a string (chunk of memory).
struct vec
{
    const char* ptr;
    size_t len;
};

// Structure used by mg_stat() function. Uses 64 bit file length.
struct mgstat {
    int is_directory;  // Directory marker
    int64_t size;      // File size
    time_t mtime;      // Modification time
};

// Describes listening socket, or socket which was accept()-ed by the master
// thread and queued for future handling by the worker thread.
struct socket {
    struct socket* next;  // Linkage
#ifdef _WIN32
    SOCKET sock;          // Listening socket
#else
    int sock;          // Listening sockete
#endif
    union usa lsa;        // Local socket address
    union usa rsa;        // Remote socket address
    int is_ssl;           // Is socket SSL-ed
};



#ifdef _WIN32
typedef HANDLE pthread_mutex_t;
typedef struct { HANDLE signal, broadcast; } pthread_cond_t;
typedef DWORD pthread_t;
#define pid_t HANDLE // MINGW typedefs pid_t to int. Using #define here.
#endif


struct mg_context {
    volatile int stop_flag;       // Should we stop event loop
    SSL_CTX* ssl_ctx;             // SSL context
    SSL_CTX* client_ssl_ctx;      // Client SSL context
    char* config[NUM_OPTIONS];    // Mongoose configuration parameters
    mg_callback_t user_callback;  // User-defined callback function
    void* user_data;              // User-defined data

    std::atomic<int> free_threads = 0;

    struct socket* listening_sockets;

    volatile int num_threads;  // Number of threads
    pthread_mutex_t mutex;     // Protects (max|num)_threads
    pthread_cond_t  cond;      // Condvar for tracking workers terminations

    struct socket queue[20];   // Accepted sockets
    volatile int sq_head;      // Head of the socket queue
    volatile int sq_tail;      // Tail of the socket queue
    pthread_cond_t sq_full;    // Singaled when socket is produced
    pthread_cond_t sq_empty;   // Signaled when socket is consumed
};

struct mg_connection {
    struct mg_request_info request_info;
    struct mg_context* ctx;
    SSL* ssl;                   // SSL descriptor
    struct socket client;       // Connected client
    time_t birth_time;          // Time connection was accepted
    int64_t num_bytes_sent;     // Total bytes sent to client
    int64_t content_len;        // Content-Length header value
    int64_t consumed_content;   // How many bytes of content is already read
    char* buf;                  // Buffer for received data
    char* path_info;            // PATH_INFO part of the URL
    int must_close;             // 1 if connection must be closed
    int buf_size;               // Buffer size
    int request_len;            // Size of the request + headers in a buffer
    int data_len;               // Total size of data in a buffer
};




// Start web server.
//
// Parameters:
//   callback: user defined event handling function or NULL.
//   options: NULL terminated list of option_name, option_value pairs that
//            specify Mongoose configuration parameters.
//
// Side-effects: on UNIX, ignores SIGCHLD and SIGPIPE signals. If custom
//    processing is required for these, signal handlers must be set up
//    after calling mg_start().
//
//
// Example:
//   const char *options[] = {
//     "document_root", "/var/www",
//     "listening_ports", "80,443s",
//     NULL
//   };
//   struct mg_context *ctx = mg_start(&my_func, NULL, options);
//
// Please refer to http://code.google.com/p/mongoose/wiki/MongooseManual
// for the list of valid option and their possible values.
//
// Return:
//   web server context, or NULL on error.
struct mg_context *mg_start(void (*user_callback)(enum mg_event event, struct mg_connection* conn), void *user_data,
                            const char **options);


int  mg_get_free_worker_count(struct mg_context* ctx);
void mg_increase_worker_count(struct mg_context* ctx, int amount);


// Stop the web server.
//
// Must be called last, when an application wants to stop the web server and
// release all associated resources. This function blocks until all Mongoose
// threads are stopped. Context pointer becomes invalid.
void mg_stop(struct mg_context *);


// Get the value of particular configuration parameter.
// The value returned is read-only. Mongoose does not allow changing
// configuration at run time.
// If given parameter name is not valid, NULL is returned. For valid
// names, return value is guaranteed to be non-NULL. If parameter is not
// set, zero-length string is returned.
const char *mg_get_option(const struct mg_context *ctx, const char *name);


// Return array of strings that represent valid configuration options.
// For each option, a short name, long name, and default value is returned.
// Array is NULL terminated.
const char **mg_get_valid_option_names(void);


// Add, edit or delete the entry in the passwords file.
//
// This function allows an application to manipulate .htpasswd files on the
// fly by adding, deleting and changing user records. This is one of the
// several ways of implementing authentication on the server side. For another,
// cookie-based way please refer to the examples/chat.c in the source tree.
//
// If password is not NULL, entry is added (or modified if already exists).
// If password is NULL, entry is deleted.
//
// Return:
//   1 on success, 0 on error.
int mg_modify_passwords_file(const char *passwords_file_name,
                             const char *domain,
                             const char *user,
                             const char *password);


// Return mg_request_info structure associated with the request.
// Always succeeds.
const struct mg_request_info *mg_get_request_info(struct mg_connection *);


// Send data to the client.
int64_t mg_write(struct mg_connection *, const void *buf, size_t len);


// Send data to the browser using printf() semantics.
//
// Works exactly like mg_write(), but allows to do message formatting.
// Note that mg_printf() uses internal buffer of size IO_BUF_SIZE
// (8 Kb by default) as temporary message storage for formatting. Do not
// print data that is bigger than that, otherwise it will be truncated.
int64_t mg_printf(struct mg_connection *, const char *fmt, ...)
#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
;


// Send contents of the entire file together with HTTP headers.
void mg_send_file(struct mg_connection *conn, const char *path);


// Read data from the remote end, return number of bytes read.
int mg_read(struct mg_connection *, void *buf, size_t len);


// Get the value of particular HTTP header.
//
// This is a helper function. It traverses request_info->http_headers array,
// and if the header is present in the array, returns its value. If it is
// not present, NULL is returned.
const char *mg_get_header(const struct mg_connection *, const char *name);


// Get a value of particular form variable.
//
// Parameters:
//   data: pointer to form-uri-encoded buffer. This could be either POST data,
//         or request_info.query_string.
//   data_len: length of the encoded data.
//   var_name: variable name to decode from the buffer
//   buf: destination buffer for the decoded variable
//   buf_len: length of the destination buffer
//
// Return:
//   On success, length of the decoded variable.
//   On error, -1 (variable not found, or destination buffer is too small).
//
// Destination buffer is guaranteed to be '\0' - terminated. In case of
// failure, dst[0] == '\0'.
int mg_get_var(const char *data, size_t data_len,
               const char *var_name, char *buf, size_t buf_len);

// Fetch value of certain cookie variable into the destination buffer.
//
// Destination buffer is guaranteed to be '\0' - terminated. In case of
// failure, dst[0] == '\0'. Note that RFC allows many occurrences of the same
// parameter. This function returns only first occurrence.
//
// Return:
//   On success, value length.
//   On error, 0 (either "Cookie:" header is not present at all, or the
//   requested parameter is not found, or destination buffer is too small
//   to hold the value).
int mg_get_cookie(const struct mg_connection *,
                  const char *cookie_name, char *buf, size_t buf_len);


// Connect to the remote web server.
// Return:
//   On success, valid pointer to the new connection
//   On error, NULL
struct mg_connection *mg_connect(struct mg_context *ctx,
                                 const char *host, int port, int use_ssl);


// Close the connection opened by mg_connect().
void mg_close_connection(struct mg_connection *conn);


// Download given URL to a given file.
//   url: URL to download
//   path: file name where to save the data
//   request_info: pointer to a structure that will hold parsed reply headers
//   buf, bul_len: a buffer for the reply headers
// Return:
//   On error, NULL
//   On success, opened file stream to the downloaded contents. The stream
//   is positioned to the end of the file. It is a user responsibility
//   to fclose() opened file stream.
FILE *mg_fetch(struct mg_context *ctx, const char *url, const char *path,
               char *buf, size_t buf_len, struct mg_request_info *request_info);


// Convenience function -- create detached thread.
// Return: 0 on success, non-0 on error.
typedef void * (*mg_thread_func_t)(void *);
int mg_start_thread(mg_thread_func_t f, void *p);


// Return builtin mime type for the given file name.
// For unrecognized extensions, "text/plain" is returned.
const char *mg_get_builtin_mime_type(const char *file_name);


// Return Mongoose version.
const char *mg_version(void);


// MD5 hash given strings.
// Buffer 'buf' must be 33 bytes long. Varargs is a NULL terminated list of
// asciiz strings. When function returns, buf will contain human-readable
// MD5 hash. Example:
//   char buf[33];
//   mg_md5(buf, "aa", "bb", NULL);
void mg_md5(char *buf, ...);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MONGOOSE_HEADER_INCLUDED