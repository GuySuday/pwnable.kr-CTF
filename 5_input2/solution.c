#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT2_PATH ("/home/input2/input2")

#define CHECK_SYSTEM_CALL(fn)                                                  \
  do {                                                                         \
    if (-1 == (fn)) {                                                          \
      perror(#fn);                                                             \
      return -1;                                                               \
    }                                                                          \
  } while (0)

#define CHECK_CALL(fn)                                                         \
  do {                                                                         \
    if (-1 == (fn)) {                                                          \
      fprintf(stderr, "Failed to run: %s\n", #fn);                             \
      return -1;                                                               \
    }                                                                          \
  } while (0)

// Returns 0 on success, -1 on error
int write_exact(int fd, const void *buffer, size_t count) {
  const char *buf = buffer;
  size_t total_written = 0;

  while (total_written < count) {
    ssize_t bytes = write(fd, buf + total_written, count - total_written);
    if (bytes < 0) {
      if (errno == EINTR) {
        continue; // Retry on signal
      }
      return -1; // Other errors
    }
    if (bytes == 0) {
      return -1; // Unexpected: treat as failure
    }
    total_written += bytes;
  }

  return 0;
}

// Returns 0 on success, -1 on error or EOF before full read
int read_exact(int fd, void *buffer, size_t count) {
  char *buf = buffer;
  size_t total_read = 0;

  while (total_read < count) {
    ssize_t bytes = read(fd, buf + total_read, count - total_read);
    if (bytes < 0) {
      if (errno == EINTR) {
        continue; // Retry on signal
      }
      return -1; // Error
    }
    if (bytes == 0) {
      return -1; // EOF before full read
    }
    total_read += bytes;
  }

  return 0;
}

int main() {
  // ============ Stage 1: argv ============
  char *argv[101] = {0};
  for (int i = 0; i < 100; ++i) {
    argv[i] = "i don't care";
  }

  argv['A'] = "\x00";
  argv['B'] = "\x20\x0a\x0d";
  // Part of stage 5
  argv['C'] = "1234";

  // ============ Stage 3: envp ============
  char *envp[2] = {0};
  envp[0] = "\xde\xad\xbe\xef=\xca\xfe\xba\xbe";

  // ============ Stage 4: file ============
  FILE *fp = fopen("\x0a", "w");

  if (NULL == fp) {
    perror("Failed to fopen");
    return -1;
  }

  char write_buffer[] = "\x00\x00\x00\x00";
  if (1 != fwrite(write_buffer, 4, 1, fp)) {
    fprintf(stderr, "Failed to fwrite of items\n");
    return -1;
  }

  CHECK_SYSTEM_CALL(fclose(fp));

  // ============ Stage 2: stdin ============
  int pipefd_stdin[2];
  int pipefd_stderr[2];
  int pid = 0;

  // Creating the pipes
  CHECK_SYSTEM_CALL(pipe(pipefd_stdin));
  CHECK_SYSTEM_CALL(pipe(pipefd_stderr));

  CHECK_SYSTEM_CALL(pid = fork());

  if (0 == pid) {
    // Child process
    // Closing the each pipe's unused write-end
    CHECK_SYSTEM_CALL(close(pipefd_stdin[1]));
    CHECK_SYSTEM_CALL(close(pipefd_stderr[1]));

    // Duplicating the pipe's read fd to fd 0 (replacing stdin)
    CHECK_SYSTEM_CALL(dup2(pipefd_stdin[0], STDIN_FILENO));
    // Duplicating the pipe's read fd to fd 2 (replacing stderr)
    CHECK_SYSTEM_CALL(dup2(pipefd_stderr[0], STDERR_FILENO));

    // This is the key to all of the stages - running the binary itself with the
    // various inputs
    CHECK_SYSTEM_CALL(execve(INPUT2_PATH, argv, envp));
    exit(0);
  } else {
    // Parent process
    // Closing each pipe's unused read-end
    CHECK_SYSTEM_CALL(close(pipefd_stdin[0]));
    CHECK_SYSTEM_CALL(close(pipefd_stderr[0]));

    // Writing data to the stdin of the child
    char stdin_buffer[] = "\x00\x0a\x00\xff";
    CHECK_SYSTEM_CALL(
        write_exact(pipefd_stdin[1], stdin_buffer, sizeof(stdin_buffer)));

    // Writing data to the stderr of the child
    char stderr_buffer[] = "\x00\x0a\x02\xff";
    CHECK_SYSTEM_CALL(
        write_exact(pipefd_stderr[1], stderr_buffer, sizeof(stderr_buffer)));

    // Wait for the a bit to let the child accept connections
    sleep(1);

    // ============ Stage 5: network ============
    int socket_fd, cd;
    struct sockaddr_in saddr, caddr;
    CHECK_SYSTEM_CALL(socket_fd = socket(AF_INET, SOCK_STREAM, 0));

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);

    CHECK_SYSTEM_CALL(
        connect(socket_fd, (struct sockaddr *)&saddr, sizeof(saddr)));

    char send_data[] = "\xde\xad\xbe\xef";

    CHECK_SYSTEM_CALL(write_exact(socket_fd, send_data, sizeof(send_data)));

    // Wait for the child to finish
    wait(NULL);
  }

  return 0;
}