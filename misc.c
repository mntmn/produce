// http://stackoverflow.com/a/14530993
void urldecode2(char *dst, const char *src)
{
  char a, b;
  while (*src) {
    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a'-'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a'-'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16*a+b;
      src+=3;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

// http://stackoverflow.com/a/646254
char* get_command_output(char* cmd) {
  /* Open the command for reading. */
  FILE* fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command %s\n",cmd);
    exit(1);
  }

  char* out=(char*)malloc(512);
  memset(out,0,512);

  /* Read the output a line at a time - output it. */
  if (fgets(out, 511, fp) != NULL) {
    pclose(fp);
    return out;
  }

  free(out);
  return NULL;
}
