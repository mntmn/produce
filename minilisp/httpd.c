#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include "minilisp.h"

#define PORT    5553
#define MAXMSG  512

#define VERB_GET 0
#define VERB_POST 1

extern char* http_to_lisp(int http_verb, char* path, char* body, int body_size);

int read_from_client (int filedes)
{
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read(filedes, buffer, MAXMSG);
  if (nbytes < 0)
  {
    // read error
    perror("httpd read");
    exit(EXIT_FAILURE);
  }
  else if (nbytes == 0) {
    // EOF
    return -1;
  }
  else
  {
    // data read
    //fprintf(stderr, "[httpd`%s']\n", buffer);
    int verb = VERB_GET;
    
    int o1 = strstr(buffer, "GET /") - buffer;
    if (o1<0 || o1>=nbytes) {
      o1 = strstr(buffer, "POST /") - buffer;
      verb = VERB_POST;
    }
    
    int o2 = strstr(buffer, " HTTP/1.1") - buffer;
    int o3 = strstr(buffer, "\r\n\r\n") - buffer;
    
    if (o1>=0 && o1<nbytes && o2>=1 && (o2-o1)<1024 && o3>o2) {
      char* path_buffer = malloc(1024);
      //printf("\r\no1: %d o2: %d\r\n",o1,o2);
      memset(path_buffer, 0, 1024);
      
      if (verb==VERB_POST) {
        strncpy(path_buffer, buffer+o1+5, o2-(o1+5));
        printf("\r\n[httpd post path: %s]\r\n",path_buffer);
      } else {  
        strncpy(path_buffer, buffer+o1+4, o2-(o1+4));
        printf("\r\n[httpd get path: %s]\r\n",path_buffer);
      }
      
      char* response = http_to_lisp(verb, path_buffer, buffer+o3, nbytes-o3+4);
      write(filedes, response, strlen(response));
      free(path_buffer);
    }
    
    return -1;
  }
}

int make_socket (uint16_t port)
{
  int sock;
  struct sockaddr_in name;

  /* Create the socket. */
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons(port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0)
  {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  return sock;
}

static int sock;
static fd_set active_fd_set, read_fd_set;

void httpd_listen()
{
  int i;
  socklen_t size;
  struct sockaddr_in clientname;

  struct timeval tv;
  fd_set readfds;

  tv.tv_sec = 0;
  tv.tv_usec = 10000;

  /* Block until input arrives on one or more active sockets. */
  read_fd_set = active_fd_set;
  if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv) < 0)
  {
    perror("httpd select");
    //exit(EXIT_FAILURE);
  }

  /* Service all the sockets with input pending. */
  for (i = 0; i < FD_SETSIZE; ++i) {
    if (FD_ISSET(i, &read_fd_set))
    {
      if (i == sock)
      {
        /* Connection request on original socket. */
        int new;
        size = sizeof(clientname);
        new = accept(sock,
                      (struct sockaddr *) &clientname,
                      &size);
        if (new < 0)
        {
          perror("httpd accept");
          exit (EXIT_FAILURE);
        }
        /*fprintf(stderr,
                 "Server: connect from host %s, port %hd.\n",
                 inet_ntoa (clientname.sin_addr),
                 ntohs(clientname.sin_port));*/
        FD_SET(new, &active_fd_set);
      }
      else
      {
        /* Data arriving on an already-connected socket. */
        if (read_from_client(i) < 0)
        {
          close(i);
          FD_CLR(i, &active_fd_set);
        }
      }
    }
  }
}

int httpd_init(void)
{
  sock = make_socket(PORT);
  
  fcntl(sock, F_SETFL, O_NONBLOCK);

  if(listen (sock, 1) < 0)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }

  // Initialize the set of active sockets.
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);
}

void httpd_close()
{
  int res = shutdown(sock, 2);
  printf("\r\nhttpd closed: %d\r\n",res);
}
