#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h> 

typedef struct text_ {
  char *txt;
  size_t cap;
  size_t used;
} text;

#define text_used(t) (t)->used

/* Zero terminate string. */
#define TERM(t) (t)->txt[(t)->cap] = 0

/* Space available. */
#define AVAIL(t) ((t)->cap - (t)->used)

text * text_new(size_t cap) {
  text *t;
  t = malloc(sizeof (text));
  if (!t) return NULL;
  t->txt = malloc(cap + 1);
  if (!t->txt) {
    free(t);
    return NULL;
  }
  memset(t->txt, 0, cap + 1);
  t->cap = cap;
  t->used = 0;
  return t;
}

text * text_from_str(const char *s) {
  text *t;
  size_t newcap;
  assert(s);
  newcap = strlen(s);
  t = text_new(newcap);
  if (!t) return NULL;
  memmove(t->txt, s, newcap);
  t->used = newcap;
  return t;
}

text * text_dup(text *t) {
  text *nt;
  nt = text_new(t->cap);
  memmove(nt->txt, t->txt, t->cap);
  TERM(nt);
  return nt;
}
  
text * text_free(text *t) {
  assert(t);
  free(t->txt);
  free(t);
  return NULL;
}

int text_resize(text *t, size_t newcap) {
  assert(t);
  t->txt = realloc(t->txt, newcap + 1);
  if (t->txt == NULL) return 0;
  t->used = (t->used < newcap) ? t->used : newcap;
  t->cap = newcap;
  return 1;
}

void text_append_buf(text *t, char *b, size_t n) {
  int rc;
  if (AVAIL(t) < n) {
    rc = text_resize(t, t->cap + n - AVAIL(t));
    if (!rc) return;
  }
  memmove(&t->txt[t->used], b, n);
  t->used += n;
  TERM(t);
}

#define BUF_SIZE 128
int text_readline(int fd, text **rv) {
  char buf[BUF_SIZE];
  text *t;

  assert(fd >= 0);  

  t = text_new(BUF_SIZE);
  for (;;) {
    ssize_t sz;
    char *nl;
    sz = read(fd, buf, BUF_SIZE);
    if (sz < 0) break;
    if (sz == 0) {
      if (errno == 0) {
	*rv = t;
	return 0;
      } else {
	break;
      }
    }
    nl = memchr(buf, '\n', sz);
    if (nl == NULL) {
      text_append_buf(t, buf, sz);
    } else {
      off_t off;
      text_append_buf(t, buf, nl - buf);
      printf("%d\n", -1 * (sz - ((nl - buf) + 1)));
      off = lseek(fd, -1 * (sz - ((nl - buf) + 1)), SEEK_CUR);
      fsync(fd);
      if (off < 0) {
	text_free(t);
	*rv = NULL;
	perror("lseek()");
	break;
      }
      *rv = t;
      return 1;
    }    
  }
  text_free(t);
  return -1;
}

int main(int argc, char **argv) {
  text *t;
  int fd;
  ssize_t rc;
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) exit(1);
  for (;;) {
    t = NULL;
    rc = text_readline(fd, &t);
    if (rc < 0) break;
    assert(t != NULL);
    printf("used=%zi '%s'\n", text_used(t), t->txt);
    text_free(t);
    if (rc == 0) break;
  }
  close(fd);
  return 0;
}
