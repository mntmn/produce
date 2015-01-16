#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#ifndef MINILISP_H
#define MINILISP_H

#define uint unsigned int
#define integer long long int
#define byte unsigned char

#define TAG_INT  0
#define TAG_CONS 1
#define TAG_SYM  2
#define TAG_LAMBDA  3
#define TAG_BUILTIN 4
#define TAG_BIGNUM 5
#define TAG_STR 6
#define TAG_BYTES 7
#define TAG_VEC 8
#define TAG_ERROR 9
#define TAG_LET 10

#define NUM_BUILTINS 30
#define BUILTIN_ADD 1
#define BUILTIN_SUB 2
#define BUILTIN_MUL 3
#define BUILTIN_DIV 4
#define BUILTIN_MOD 5
#define BUILTIN_LT 6
#define BUILTIN_GT 7
#define BUILTIN_EQ 8
#define BUILTIN_DO 9
#define BUILTIN_DEF 10
#define BUILTIN_IF  11
#define BUILTIN_FN  12
#define BUILTIN_LIST  13
#define BUILTIN_CAR 14
#define BUILTIN_CDR 15
#define BUILTIN_CONS 16
#define BUILTIN_GET 17
#define BUILTIN_SET 18
#define BUILTIN_LEN 19
#define BUILTIN_TYPE 20
#define BUILTIN_LET 21
#define BUILTIN_QUOTE 22
#define BUILTIN_MAP 23
#define BUILTIN_FILTER 24
#define BUILTIN_CONCAT 25
#define BUILTIN_SUBSTR 26
#define BUILTIN_APPEND 27
#define BUILTIN_REVERSE 28
#define BUILTIN_EVAL 29
#define BUILTIN_LOAD 30
#define BUILTIN_STR 31
#define BUILTIN_DEBUG 32
#define BUILTIN_READ 33
#define BUILTIN_ALIEN 34
#define BUILTIN_SAVE 35

#define MAX_EVAL_DEPTH 10000
#define SYM_INIT_BUFFER_SIZE 8
#define BIGNUM_INIT_BUFFER_SIZE 32
#define BIGNUM_THRESHOLD 922337203685477580 // 2^63/10

#define ERR_SYNTAX 0
#define ERR_MAX_EVAL_DEPTH 1
#define ERR_UNKNOWN_OP 2
#define ERR_APPLY_NIL 3
#define ERR_INVALID_PARAM_TYPE 4
#define ERR_OUT_OF_BOUNDS 5

#define ERR_NOT_FOUND 404
#define ERR_FORBIDDEN 403

#define FLAG_FREED 1

#define min(a,b) (a > b ? a : b)
#define max(a,b) (b > a ? a : b)

typedef struct Cell {
  uint tag;
  union {
    integer value;
    void* addr;
  };
  uint size;
  void* next;
  uint scope_id;
  char flags;
} Cell;

int is_nil(Cell* c);

typedef Cell* (*alien_func)(Cell* args, Cell* env);

Cell* car(Cell* c);
Cell* cdr(Cell* c);
Cell* apply(Cell* lambda, Cell* args, Cell* env);
Cell* eval(Cell* expr, Cell* env);
Cell* alloc_cons(Cell* ar, Cell* dr);
Cell* alloc_sym(char* str);
Cell* alloc_bignum(integer i);
Cell* alloc_bytes();
Cell* alloc_num_bytes(integer num_bytes);
Cell* alloc_string();
Cell* alloc_int(integer i);
Cell* alloc_nil();
Cell* alloc_error(uint code);
Cell* alloc_lambda(Cell* def);
Cell* alloc_let(Cell* def);
Cell* alloc_clone(Cell* orig, uint scope_id);
Cell* append(Cell* cell, Cell* list);
void  free_tree(Cell* root);
void  enter_scope();
void  exit_scope();
char* lisp_write(Cell* cell, char* buffer, int bufsize);
Cell* get_globals();
void  register_alien_func(char* symname, alien_func func);

void  print_memstats();
void  print_globals();
void  reset_counters();
void  debug_cell(Cell* c);

void  init_lisp();

extern Cell* read_string(char* in);

#endif
