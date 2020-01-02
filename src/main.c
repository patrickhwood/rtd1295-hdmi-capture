#include "v4l2_driver.h"
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define SAVE_EVERY_FRAME 0

pthread_t thread_stream;
void *sdlScreen;
void *sdlRenderer;
void *sdlTexture;

/* miscellanous */
int thread_exit_sig = 0;

struct streamHandler {
  int fd;
  void (*framehandler)(void *pframe, int length);
};

void print_help() {
  printf("Usage: cap <width> <height> <device>\n");
  printf("Example: cap 640 480 /dev/video0\n");
}

static void frame_handler(void *pframe, int length) {
  static int yuv_index = 0;
  char yuvifle[100];
  sprintf(yuvifle, "yuv-%d.yuv", yuv_index);
  FILE *fp = fopen(yuvifle, "wb");
  fwrite(pframe, length, 1, fp);
  fclose(fp);
  yuv_index++;
}

static void *v4l2_streaming(void *arg) {
  int fd = ((struct streamHandler *)(arg))->fd;
  void (*handler)(void *pframe, int length) =
      ((struct streamHandler *)(arg))->framehandler;

  fd_set fds;
  struct v4l2_buffer buf;
  while (!thread_exit_sig) {
    int ret;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (-1 == ret) {
      fprintf(stderr, "select error\n");
      return NULL;
    } else if (0 == ret) {
      fprintf(stderr, "timeout waiting for frame\n");
      continue;
    }
    if (FD_ISSET(fd, &fds)) {
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
        fprintf(stderr, "VIDIOC_DQBUF failure\n");
        return NULL;
      }
#ifdef DEBUG
      printf("deque buffer %d\n", buf.index);
#endif

      if (handler)
        (*handler)(v4l2_ubuffers[buf.index].start,
                   v4l2_ubuffers[buf.index].length);

      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
        fprintf(stderr, "VIDIOC_QBUF failure\n");
        return NULL;
      }
#ifdef DEBUG
      printf("queue buffer %d\n", buf.index);
#endif
    }
  }
  return NULL;
}

int main(int argc, char const *argv[]) {
  const char *device = "/dev/video0";

  if (argc == 2 && (strchr(argv[1], 'h') != NULL)) {
    print_help();
    exit(0);
  }

  if (argc > 2) {
    IMAGE_WIDTH = atoi(argv[1]);
    IMAGE_HEIGHT = atoi(argv[2]);
  }

  if (argc > 3) {
    device = argv[3];
  }

  int video_fildes = v4l2_open(device);
  if (video_fildes == -1) {
    fprintf(stderr, "can't open %s\n", device);
    exit(-1);
  }

  if (v4l2_querycap(video_fildes, device) == -1) {
    perror("v4l2_querycap");
    goto exit_;
  }

  // most of devices support YUYV422 packed.
  if (v4l2_sfmt(video_fildes, V4L2_PIX_FMT_YUYV) == -1) {
    perror("v4l2_sfmt");
    goto exit_;
  }

  if (v4l2_gfmt(video_fildes) == -1) {
    perror("v4l2_gfmt");
    goto exit_;
  }

  if (v4l2_sfps(video_fildes, 30) == -1) { // no fatal error
    perror("v4l2_sfps");
  }

  if (v4l2_mmap(video_fildes) == -1) {
    perror("v4l2_mmap");
    goto exit_;
  }

  if (v4l2_streamon(video_fildes) == -1) {
    perror("v4l2_streamon");
    goto exit_;
  }

  // create a thread that will update frame int the buffer
  struct streamHandler sH = {video_fildes, frame_handler};
  if (pthread_create(&thread_stream, NULL, v4l2_streaming, (void *)(&sH))) {
    fprintf(stderr, "create thread failed\n");
    goto exit_;
  }
  sleep(1);

  thread_exit_sig = 1;               // exit thread_stream
  pthread_join(thread_stream, NULL); // wait for thread_stream exiting

  if (v4l2_streamoff(video_fildes) == -1) {
    perror("v4l2_streamoff");
    goto exit_;
  }

  if (v4l2_munmap() == -1) {
    perror("v4l2_munmap");
    goto exit_;
  }

exit_:
  if (v4l2_close(video_fildes) == -1) {
    perror("v4l2_close");
  };
  return 0;
}
