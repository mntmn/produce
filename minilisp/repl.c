#include "minilisp.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void run_tests() {
  //run_lisp_tests();
  //run_reader_tests();
  //run_eval_tests();
}

#define REPL_BUFFER_SIZE 1024*1024


static struct termios   save_termios;
static int              term_saved;

// from http://www.lafn.org/~dave/linux/terminalIO.html
int tty_raw(int fd) {       /* RAW! mode */
  struct termios  buf;

  if (tcgetattr(fd, &save_termios) < 0) /* get the original state */
    return -1;

  buf = save_termios;

  buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  /* echo off, canonical mode off, extended input
     processing off, signal chars off */

  buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
  /* no SIGINT on BREAK, CR-toNL off, input parity
     check off, don't strip the 8th bit on input,
     ouput flow control off */

  buf.c_cflag &= ~(CSIZE | PARENB);
  /* clear size bits, parity checking off */

  buf.c_cflag |= CS8;
  /* set 8 bits/char */

  buf.c_oflag &= ~(OPOST);
  /* output processing off */

  buf.c_cc[VMIN] = 1;  /* 1 byte at a time */
  buf.c_cc[VTIME] = 0; /* no timer on input */

  if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
    return -1;

  term_saved = 1;

  return 0;
}

int tty_reset(int fd) { /* set it to normal! */
    if (term_saved)
        if (tcsetattr(fd, TCSAFLUSH, &save_termios) < 0)
            return -1;

    return 0;
}

void cleanup() {
  tty_reset(STDIN_FILENO);
  httpd_close();
}

extern int httpd_init();
extern void http_close();

#define HTTP_RES_BUF_SIZE 64*1024
static char http_res_buf[HTTP_RES_BUF_SIZE];

char* http_to_lisp(int http_verb, char* path, char* body, int body_size) {
  memset(http_res_buf, 0, HTTP_RES_BUF_SIZE);

  Cell* path_cell = alloc_num_bytes(strlen(path)+1);
  path_cell->tag = TAG_STR;
  strncpy(path_cell->addr,path,path_cell->size);
  
  Cell* body_cell;

  if (body_size>0) {
    body_cell = alloc_num_bytes(body_size+1);
    body_cell->tag = TAG_STR;
    strncpy(body_cell->addr,body,body_cell->size);
  } else {
    body_cell = alloc_nil();
  }
  
  Cell* http_expr;

  if (http_verb == 1) {
    http_expr = alloc_cons(alloc_sym("httpd-post"),alloc_cons(path_cell,alloc_cons(body_cell,0)));
  } else {
    http_expr = alloc_cons(alloc_sym("httpd-get"),alloc_cons(path_cell,alloc_cons(body_cell,0)));
  }
  
  //printf("\r\nexpr_buf: [%s]\r\n\n",expr_buf);

  Cell* http_response = eval(http_expr, get_globals());

  if (http_response && http_response->tag == TAG_STR) {
    return http_response->addr;
  } else {
    lisp_write(http_response, http_res_buf, HTTP_RES_BUF_SIZE-1);
    return http_res_buf;
  }
}

void load_boot_file() {
  char* boot_buffer = malloc(REPL_BUFFER_SIZE);
  memset(boot_buffer,0,REPL_BUFFER_SIZE);
  
  FILE* f;
  if ((f = fopen("boot.l","r"))) {
    int l = fread(boot_buffer, 1, REPL_BUFFER_SIZE, f);
    fclose(f);

    if (l) {
      printf("[boot file bytes read: %d]\r\n\n",l);
      Cell* expr = read_string(boot_buffer);
      Cell* evaled = alloc_clone(eval(expr, get_globals()), 0);
    }
  }

  free(boot_buffer);
}

int main() {
  char* write_buffer = malloc(REPL_BUFFER_SIZE);
  memset(write_buffer,0,REPL_BUFFER_SIZE);
  char* line_buffer = malloc(REPL_BUFFER_SIZE);
  size_t size;

  atexit(cleanup);
  tty_raw(STDIN_FILENO);

  init_lisp();

  printf("minilisp 0.5.0\r\n");

  load_boot_file();
  httpd_init();
  printf("mini> ");
  char in_char = 0;
  int line_count = 0;
  int running = 1;

  fd_set in_fds;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  fflush(stdout);

  while (running) {
    httpd_listen();

    FD_ZERO(&in_fds);
    FD_SET(STDIN_FILENO, &in_fds);

    select(1, &in_fds, NULL, NULL, &timeout); 
    
    if (FD_ISSET(STDIN_FILENO, &in_fds)) {
      int in_count = fread(&in_char, 1, 1, stdin);

      if (in_count>0) {
        //printf("%c",in_char);

        if (in_char == '\r') {
          printf("\r\n");
          
          line_buffer[line_count++] = '\n';
          line_buffer[line_count++] = 0;
          if (!strcmp(line_buffer,"quit\n")) exit(0);

          if (!strcmp(line_buffer,"boot\n")) {
            load_boot_file();
          } else {
            Cell* expr = read_string(line_buffer);
            Cell* evaled = alloc_clone(eval(expr, get_globals()), 0);
            lisp_write(evaled, write_buffer, REPL_BUFFER_SIZE);
            printf("%s\r\nmini> ",write_buffer);
            free_tree(expr);
            free_tree(evaled);
          }
          line_count = 0;
        }
        else if (in_char == 127) { // backspace
          line_count--;
          printf("\0x1b[2D");
        }
        else {
          //printf("[%d]",in_char);
          printf("%c",in_char);
          line_buffer[line_count++] = in_char;
        }
        fflush(stdout);
        
        /*
        //printf("%s\n", line);

        Cell* expr = read_string(line);
        //printf("expr: %p\n",expr);
        //lisp_write(expr, write_buffer);
        //printf("%s\n",write_buffer);

        Cell* evaled = alloc_clone(eval(expr, get_globals()), 0);
        //printf("evaled: %p\n",evaled);

        lisp_write(evaled, write_buffer, REPL_BUFFER_SIZE);
        printf("%s\n",write_buffer);

        free_tree(expr);
        free_tree(evaled);*/
      }
    } else {
      //printf(".");
    }
  }
  return 0;
}
