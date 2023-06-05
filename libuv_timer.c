#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define TIMER_SET 5;  // duration in seconds

/*Error handling macro*/
#define CHECK(r, msg)                                                    \
  if (r < 0) {                                                           \
    fprintf(stderr, "%s: [%s(%d): %s]\n", msg, uv_err_name((r)), (int)r, \
            uv_strerror((r)));                                           \
    exit(1);                                                             \
  }

int err;
uv_loop_t *loop;

void timer_cb(uv_timer_t *timer) {
  static int duration = TIMER_SET;

  if (duration >= 0) {
    printf("Countdown: %d\n", duration);
    --duration;
  } else {
    uv_close((uv_handle_t *)timer, NULL);
  }
}

int main(void) {
  loop = uv_default_loop();

  uv_timer_t timer;

  err = uv_timer_init(loop, &timer);
  CHECK(err, "uv_timer_init");

  err = uv_timer_start(&timer, timer_cb, 0, 1000);
  CHECK(err, "uv_timer_start");

  uv_run(loop, UV_RUN_DEFAULT);

  err = uv_loop_close(loop);
  CHECK(err, "uv_loop_close");

  return 0;
}