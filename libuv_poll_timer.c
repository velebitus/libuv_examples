#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define MAX_WATCH 4        // max number of files being watched
#define MAX_TIME 1000      // max duration of timer in seconds
#define POLL_EVERY_MS 200  // milisecond

/*Error handling macro*/
#define CHECK(r, msg)                                                    \
  if (r < 0) {                                                           \
    fprintf(stderr, "%s: [%s(%d): %s]\n", msg, uv_err_name((r)), (int)r, \
            uv_strerror((r)));                                           \
    exit(1);                                                             \
  }

int err;
uv_loop_t *loop;

void walk_cb(uv_handle_t *handle, void *arg) {
  /*Close handle if running*/
  if (!uv_is_closing(handle)) {
    uv_close(handle, NULL);
  }
}

void timer_cb(uv_timer_t *timer) {
  err = uv_timer_stop(timer);
  CHECK(err, "uv_timer_stop");

  uv_stop(loop);
}

/*Print file that changed*/
void on_file_change(uv_fs_poll_t *handle, int status, const uv_stat_t *prev,
                    const uv_stat_t *curr) {
  CHECK(status, "on_file_change");

  printf("File %s has been modified\n", (const char *)handle->data);
}

int main(int argc, char **argv) {
  /*Alert user if entered more than allowed files*/
  if (argc > (MAX_WATCH + 1)) {
    fprintf(stderr, "More files have been entered than allowed.");
    fprintf(stderr, "The program will not watch all of the files.\n");
  }

  /*Get timer duration*/
  char string[100];
  unsigned int duration = 0;
  do {
    printf("How long do you wish to watch specified files? In seconds[s]: ");
    fgets(string, sizeof(string), stdin);
    sscanf(string, " %u", &duration);
  } while (duration < 1 || duration > MAX_TIME);

  loop = uv_default_loop();

  /*Create handle for each path*/
  uv_fs_poll_t *poll_handles[MAX_WATCH];
  int k;
  for (int i = 1; (i < argc) && (i <= MAX_WATCH); i++) {
    k = i - 1;  // for poll_handles index

    const char *file_path = argv[i];
    poll_handles[k] = malloc(sizeof(uv_fs_poll_t));

    err = uv_fs_poll_init(loop, poll_handles[k]);
    CHECK(err, "uv_fs_poll_init");

    poll_handles[k]->data = (void *)file_path;

    /*Poll files for changes every POLL_EVERY_MS*/
    err = uv_fs_poll_start(poll_handles[k], on_file_change, file_path,
                           POLL_EVERY_MS);
    CHECK(err, "uv_fs_poll_start");

    printf("Watching file %s for changes\n", file_path);
  }

  uv_timer_t timer;
  uv_timer_init(loop, &timer);

  char decision;      // in while below
  bool enter = true;  // used to enter while below
go_to_run_again:      // goto in switch below

  uv_update_time(loop);
  /*Exit process after set time*/
  err = uv_timer_start(&timer, timer_cb, (duration * 1000), 0);
  CHECK(err, "uv_timer_start");
  uv_run(loop, UV_RUN_DEFAULT);

  /*Get user input, decide if program continues*/
  /*Not optimized, only for demonstration*/
  while (enter) {
    printf(
        "Do you wish to continue watching the files for another %d seconds? "
        "[y\\n]: ",
        duration);
    fgets(string, sizeof(string), stdin);
    sscanf(string, " %c", &decision);

    switch (decision) {
      case 'y':
        /*fall-through*/
      case 'Y':
        goto go_to_run_again;
        break;
      case 'n':
        /*fall-through*/
      case 'N':
        enter = false;
        printf("Turning off...\n");
        break;
      default:
        printf("Wrong input...\n");
    }
  }

  /*Cleanup handles*/
  uv_walk(loop, walk_cb, NULL);
  uv_run(loop, UV_RUN_DEFAULT);

  /*Cleanup malloc*/
  for (int i = 0; i < (argc - 1) && i < MAX_WATCH; i++) {
    free(poll_handles[i]);
  }

  err = uv_loop_close(loop);
  CHECK(err, "uv_loop_close");

  return 0;
}
