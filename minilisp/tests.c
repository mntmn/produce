#include "reader.h"
#include "minilisp.h"

#define TEST_BUFSIZE 2048

void run_lisp_tests() {

  char write_buffer[TEST_BUFSIZE];

  Cell* globals = get_globals();

  globals = append(alloc_cons(alloc_sym("foo"), alloc_int(123)), globals);
  printf("globals: %s\n\n\n", write(globals, write_buffer, TEST_BUFSIZE));

  // T1------------------
  enter_scope();
  Cell* t1 = alloc_cons(alloc_sym("+"), alloc_cons(alloc_int(42), alloc_cons(alloc_int(23), alloc_nil())));
  printf("evaluating t1: %s\n", write(t1, write_buffer, TEST_BUFSIZE));
  Cell* result = eval(t1, globals);
  free_tree(t1);
  printf("t1 result: %s\n\n", write(result, write_buffer, TEST_BUFSIZE));
  free_tree(result);
  print_memstats();
  exit_scope();

  // T2------------------
  enter_scope();
  Cell* t2 = alloc_cons(alloc_sym("+"), alloc_cons(alloc_sym("foo"), alloc_cons(alloc_int(124), alloc_nil())));
  printf("evaluating t2: %s\n", write(t2, write_buffer, TEST_BUFSIZE));
  result = eval(t2, globals);
  free_tree(t2);
  printf("t2 result: %s\n\n", write(result, write_buffer, TEST_BUFSIZE));
  free_tree(result);
  print_memstats();
  exit_scope();

  // T3------------------
  enter_scope();
  // (def double (lambda (x) (+ x x)))
  Cell* t3 = alloc_cons(alloc_sym("def"),
    alloc_cons(alloc_sym("double"),
      alloc_cons(alloc_cons(alloc_sym("fn"), alloc_cons( alloc_cons(alloc_sym("x"),alloc_nil()),
        alloc_cons(alloc_cons(alloc_sym("+"), alloc_cons(alloc_sym("x"), alloc_cons(alloc_sym("x"), alloc_nil()))),alloc_nil())
      )),alloc_nil())));
  printf("evaluating t3: %s\n", write(t3, write_buffer, TEST_BUFSIZE));
  result = eval(t3, globals);
  free_tree(t3);
  printf("t3 result: %s\n\n", write(result, write_buffer, TEST_BUFSIZE));
  free_tree(result);
  print_memstats();
  print_globals();
  exit_scope();

  // T4------------------
  reset_counters();
  enter_scope();
  // (double (double foo))
  Cell* t4 = alloc_cons(alloc_sym("double"),alloc_cons(
    alloc_cons(alloc_sym("double"), alloc_cons(alloc_sym("foo"),alloc_nil())),alloc_nil()));
  printf("evaluating t4: %s\n", write(t4, write_buffer, TEST_BUFSIZE));

  time_t start = time(NULL);
  int i=0;
  for (i=0; i<1000000; i++) {
    result = eval(t4, globals);
    free_tree(result);
  }
  printf("time spent: %.2f\n",(double)(time(NULL)-start));

  free_tree(t4);
  printf("t4 result: %s\n\n", write(result, write_buffer, TEST_BUFSIZE));
  print_memstats();
  print_globals();
  exit_scope();
}

void run_reader_tests() {
  char write_buffer[TEST_BUFSIZE];

  ReaderState rs;

  char* in = "(123 (45 2345) 6)";
  rs.state = PST_ATOM;
  rs.cell = 0;
  rs.level = 0;
  rs.stack = malloc(100*sizeof(Cell*));
  Cell** root = rs.stack;

  int i=0;
  for (i=0; i<strlen(in); i++) {
    char c = in[i];
    read_char(c, &rs);

    printf("rs %c: %d %lld\n", c, rs.state, car(rs.cell)?car(rs.cell)->value:0);
  }

  write(*root, write_buffer, TEST_BUFSIZE);
  printf("read expression: %s\n",write_buffer);
}

void run_eval_tests() {
  char write_buffer[TEST_BUFSIZE];

  // T1
  Cell* expr = read_string("(+ 1234 (+ 1123423 3412343))");
  write(expr, write_buffer, TEST_BUFSIZE);
  printf("t1 read expression: %s\n",write_buffer);

  write(eval(expr, get_globals()), write_buffer, TEST_BUFSIZE);
  printf("t1 evaled expression: %s\n\n",write_buffer);

  // T2
  expr = read_string("(if (- 2 2) 0 1)");
  write(expr, write_buffer, TEST_BUFSIZE);
  printf("t2 read expression: %s\n",write_buffer);

  write(eval(expr, get_globals()), write_buffer, TEST_BUFSIZE);
  printf("t2 evaled expression: %s\n",write_buffer);

  // T3
  expr = read_string("(if (+ 0 1) 1 0)");
  write(expr, write_buffer, TEST_BUFSIZE);
  printf("t3 read expression: %s\n",write_buffer);

  write(eval(expr, get_globals()), write_buffer, TEST_BUFSIZE);
  printf("t3 evaled expression: %s\n",write_buffer);

  // T4
  expr = read_string("(if (if (if ) ) )");
  write(expr, write_buffer, TEST_BUFSIZE);
  printf("t4 read expression: %s\n",write_buffer);

  write(eval(expr, get_globals()), write_buffer, TEST_BUFSIZE);
  printf("t4 evaled expression: %s\n",write_buffer);
}