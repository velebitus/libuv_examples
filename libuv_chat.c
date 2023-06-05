#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128
#define MAX_CLIENTS 5
#define SET_TIMER 40000  // miliseconds, how long will server live

/*Error handling macro*/
#define CHECK(r, msg)                                                    \
  if (r < 0) {                                                           \
    fprintf(stderr, "%s: [%s(%d): %s]\n", msg, uv_err_name((r)), (int)r, \
            uv_strerror((r)));                                           \
    exit(1);                                                             \
  }

int err;
uv_loop_t *loop;
struct sockaddr_in addr;

uv_tcp_t *client_arr[MAX_CLIENTS];
int client_number = 0;

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void on_walk(uv_handle_t *handle, void *arg) {
  /*Close handle if running*/
  if (!uv_is_closing(handle)) {
    uv_close(handle, NULL);
  }
}

void on_timer(uv_timer_t *timer) {
  err = uv_timer_stop(timer);
  CHECK(err, "uv_timer_stop");
  uv_stop(loop);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

/*Free client handle and sort client_arr*/
void on_close_regular(uv_handle_t *client) {
  --client_number;  // client disconnected

  for (int i = 0; i < (client_number + 1); ++i) {
    if ((uv_tcp_t *)client == client_arr[i]) {
      free(client_arr[i]);  // free so it doesn't get lost
      client_arr[i] = NULL;
      /*Replace handle with last in array*/
      client_arr[i] = client_arr[client_number];
      client_arr[client_number] = NULL;  // for no unwanted behaviour
      break;
    }
  }
  fprintf(stderr, "Client disconnected! ");
  fprintf(stderr, "Number of clients: %d\n", client_number);
}

/*Free client handle that was not used*/
void on_close_nonregular(uv_handle_t *client) { free(client); }

void on_write(uv_write_t *req, int status) {
  if (status) {
    fprintf(stderr, "Write error %s\n", uv_strerror(status));
  }
  write_req_t *wr = (write_req_t *)req;
  free(wr);
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    for (int i = 0; i < client_number; ++i) {
      if (client != ((uv_stream_t *)client_arr[i])) {
        write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t *)req, (uv_stream_t *)client_arr[i], &req->buf, 1,
                 on_write);
      }
    }
    free(buf->base);
    return;
  }

  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error %s\n", uv_err_name(nread));
    }
    uv_close((uv_handle_t *)client, on_close_regular);
  }
  free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }

  uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);

  if (uv_accept(server, (uv_stream_t *)client) == 0 &&
      client_number < MAX_CLIENTS) {
    client_arr[client_number] = client;
    uv_read_start((uv_stream_t *)client_arr[client_number], alloc_buffer,
                  on_read);

    ++client_number;
    fprintf(stderr, "New connection! ");
    fprintf(stderr, "Number of clients: %d\n", client_number);
  } else {
    if (client_number == MAX_CLIENTS) {
      fprintf(stderr, "Maximum client number reached!\n");
    }
    uv_close((uv_handle_t *)client, on_close_nonregular);
  }
}

int main() {
  loop = uv_default_loop();

  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
  err = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
  CHECK(err, "uv_listen");

  uv_timer_t timer;
  uv_timer_init(loop, &timer);
  err = uv_timer_start(&timer, on_timer, SET_TIMER, 0);
  CHECK(err, "uv_timer_start");

  uv_run(loop, UV_RUN_DEFAULT);

  /*Cleanup handles*/
  uv_walk(loop, on_walk, NULL);
  uv_run(loop, UV_RUN_DEFAULT);

  /*Cleanup malloc*/
  for (int i = 0; i < client_number; i++) {
    free(client_arr[i]);
  }

  err = uv_loop_close(loop);
  CHECK(err, "uv_loop_close");

  return 0;
}
