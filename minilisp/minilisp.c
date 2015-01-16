#include "minilisp.h"
#include "bignum.h"

static Cell* stack;
static Cell* globals;

uint alloc_cons_count = 0;
uint alloc_sym_count = 0;
uint alloc_int_count = 0;
uint alloc_builtin_count = 0;
uint alloc_lambda_count = 0;

uint free_cons_count = 0;
uint free_sym_count = 0;
uint free_int_count = 0;
uint free_builtin_count = 0;
uint free_lambda_count = 0;

uint eval_depth = 0;

uint active_scope_id = 0;

Cell* get_globals() {
  return globals;
}

void print_memstats() {
  printf("allocs: %3d c | %3d s | %3d i | %3d b\n",alloc_cons_count,alloc_sym_count,alloc_int_count,alloc_builtin_count);
  printf("frees : %3d c | %3d s | %3d i | %3d b\n\n",free_cons_count,free_sym_count,free_int_count,free_builtin_count);
}

void print_globals() {
  char buf[4096];
  printf("globals: %s\n",lisp_write(globals, buf, 4096));
}

void reset_counters() {
  alloc_cons_count = 0;
  alloc_sym_count = 0;
  alloc_int_count = 0;
  alloc_builtin_count = 0;

  free_cons_count = 0;
  free_sym_count = 0;
  free_int_count = 0;
  free_builtin_count = 0;
}

void enter_scope() {
  active_scope_id++;
}

void exit_scope() {
  active_scope_id--;
}

Cell* car(Cell* c) {
  if (!c) return 0;
  return (Cell*)c->addr;
}

Cell* cdr(Cell* c) {
  if (!c) return 0;
  return (Cell*)c->next;
}

Cell* lookup_symbol(Cell* sym, Cell* env) {
  //printf("\r\n");
  Cell* env_cell = env;
  do {
    //printf("? lookup %s / %s\r\n",sym->addr,car(car(env_cell))->addr);
    if (strcmp(sym->addr, car(car(env_cell))->addr)==0) {
      //printf("! lookup %s found %s\r\n",sym->addr,car(car(env_cell))->addr);
      return cdr(car(env_cell));
    }
    env_cell = (Cell*)env_cell->next;
  } while (car(env_cell));
  return env_cell; // nil
}

Cell* append(Cell* cell, Cell* list) {
  Cell* new_list = alloc_cons(cell, list);
  new_list->scope_id = list->scope_id;
  return new_list;
}

Cell* apply_int_fold(Cell* lambda, Cell* args, Cell* env) {
  Cell* res;
  uint op = lambda->value;
  Cell* a = car(args);
  if (!a) return alloc_error(ERR_INVALID_PARAM_TYPE);

  if (a->tag != TAG_INT && a->tag != TAG_BIGNUM) {
    a = eval(a, env);
  }
  if (a->tag != TAG_INT && a->tag != TAG_BIGNUM) {
    return alloc_error(ERR_INVALID_PARAM_TYPE);
  }
  res = alloc_clone(a, active_scope_id);

  args = cdr(args);

  while ((a = car(args))) {
    if (a->tag != TAG_INT && a->tag != TAG_BIGNUM) {
      a = eval(a, env);
    }

    if (a && a->tag == TAG_INT && res->tag == TAG_INT) {
      switch (op) {
        case BUILTIN_ADD: res->value += a->value; break;
        case BUILTIN_SUB: res->value -= a->value; break;
        case BUILTIN_MUL: res->value *= a->value; break;
        case BUILTIN_DIV: res->value /= a->value; break;
        case BUILTIN_MOD: res->value %= a->value; break;
        default: break;
      }
    } else if (a && (a->tag == TAG_BIGNUM || res->tag == TAG_BIGNUM)) {
      if (res->tag != TAG_BIGNUM) {
        integer v = res->value;
        free(res);
        res = alloc_bignum(v);
      }
      switch (op) {
        case BUILTIN_ADD: bignum_add(res, a); break;
        case BUILTIN_SUB: bignum_sub(res, a); break;
        default: break;
      }
    } else {
      if (a) return alloc_error(ERR_INVALID_PARAM_TYPE);
    }
    args = cdr(args);
  };
  return res;
}

Cell* apply_compare(Cell* lambda, Cell* args, Cell* env) {
  Cell* a = eval(car(args), env);
  if (!a || (a->tag!=TAG_INT && a->tag!=TAG_BIGNUM && a->tag!=TAG_STR && a->tag!=TAG_SYM)) return alloc_error(ERR_INVALID_PARAM_TYPE);
  Cell* b = eval(car(cdr(args)), env);
  if (!b || (b->tag!=TAG_INT && b->tag!=TAG_BIGNUM && b->tag!=TAG_STR && b->tag!=TAG_SYM)) return alloc_error(ERR_INVALID_PARAM_TYPE);

  if (a->tag==TAG_STR || b->tag==TAG_STR) {
    if ((a->tag!=TAG_STR && a->tag!=TAG_SYM) || (b->tag!=TAG_STR && b->tag!=TAG_SYM)) return alloc_error(ERR_INVALID_PARAM_TYPE);

    return alloc_int(!strcmp(a->addr,b->addr));
  }

  if (a->tag==TAG_INT && b->tag==TAG_INT) {
    switch (lambda->value) {
      case BUILTIN_LT:
        return alloc_int(a->value<b->value?1:0);
      case BUILTIN_GT:
        return alloc_int(a->value>b->value?1:0);
      case BUILTIN_EQ:
      default:
        return alloc_int(a->value==b->value?1:0);
    }
  }

  int free_a = 0;
  int free_b = 0;

  if (a->tag==TAG_INT) {
    a = alloc_bignum(a->value);
    free_a = 1;
  }
  if (b->tag==TAG_INT) {
    b = alloc_bignum(b->value);
    free_b = 1;
  }

  int res = 0;
  a = bignum_sub(a,b);
  int negative = ((char*)a->addr)[0]=='-';
  int equal = ((char*)a->addr)[0]=='0';
  switch (lambda->value) {
    case BUILTIN_LT:
      res = negative?1:0;
      break;
    case BUILTIN_GT:
      res = !(negative || equal)?1:0;
      break;
    case BUILTIN_EQ:
    default:
      res = equal?1:0;
  }
  if (free_a) free_tree(a);
  if (free_b) free_tree(b);
  return alloc_int(res);
}

Cell* apply_load(Cell* lambda, Cell* args, Cell* env) {
  Cell* path = eval(car(args), env);
  if (path->tag != TAG_STR) return alloc_error(ERR_INVALID_PARAM_TYPE);

  struct stat st;
  if (stat(path->addr, &st)) return alloc_error(ERR_NOT_FOUND);

  if (st.st_size < 1) return alloc_bytes();

  FILE* f = fopen(path->addr,"rb");
  if (!f) return alloc_error(ERR_FORBIDDEN);

  Cell* result_cell = alloc_num_bytes(st.st_size);
  fread(result_cell->addr, 1, st.st_size, f);
  fclose(f);

  return result_cell;
}

Cell* apply_save(Cell* lambda, Cell* args, Cell* env) {
  Cell* path = eval(car(args), env);
  if (path->tag != TAG_STR) return alloc_error(ERR_INVALID_PARAM_TYPE);
  Cell* obj = eval(car(cdr(args)), env);
  if (obj->tag != TAG_STR && obj->tag != TAG_BYTES) return alloc_error(ERR_INVALID_PARAM_TYPE);
  
  FILE* f = fopen(path->addr,"wb");
  if (!f) return alloc_error(ERR_FORBIDDEN);

  int bytes_written = fwrite(obj->addr, 1, obj->size, f);
  fclose(f);

  return alloc_int(bytes_written);
}

Cell* apply_read(Cell* lambda, Cell* args, Cell* env) {
  Cell* str = eval(car(args), env);
  if (str->tag != TAG_STR) return alloc_error(ERR_INVALID_PARAM_TYPE);

  return read_string(str->addr);
}

Cell* apply_make_str(Cell* lambda, Cell* args, Cell* env) {
  Cell* arg = eval(car(args), env);

  if (arg->tag == TAG_STR) return arg;

  Cell* result;
  if (arg->tag == TAG_BYTES || arg->tag == TAG_SYM) {
    // cast a copy to string
    result = alloc_num_bytes(arg->size+1);
    memcpy(result->addr, arg->addr, arg->size);
    result->size = strlen(result->addr);
  } else if (arg->tag == TAG_CONS) {
    // TODO: this is really hard to figure out.
    result = alloc_num_bytes(10*1024);
    lisp_write(arg, result->addr, result->size);
    result->size = strlen(result->addr);
  } else {
    result = alloc_num_bytes(256);
    lisp_write(arg, result->addr, result->size);
    result->size = strlen(result->addr);
  }
  result->tag = TAG_STR;

  //debug_cell(result);

  return result;
}

Cell* apply_concat(Cell* args, Cell* env) {
  integer chunk_size = 1024 * 10;
  integer buf_size = chunk_size*2;
  
  Cell* res = alloc_string();
  free(res->addr);
  res->addr = malloc(buf_size);
  integer out_count = 0;

  int count = 0;
  Cell* a;
  while ((a = car(args))) {
    a = eval(a, env);
    if (count==0) {
      // if arg0 is a list, concat the entries of that list, else concat the args
      if (a && a->tag == TAG_CONS) {
        args = a;
        a = car(a);
      }
    }

    if (buf_size<(out_count + chunk_size + 1)) {
      buf_size*=2;
      res->addr = realloc(res->addr, buf_size);
    }

    if (a && a->tag == TAG_STR) {
      strncpy(res->addr+out_count, a->addr, min(chunk_size,res->size));
    } else {
      lisp_write(a, res->addr+out_count, chunk_size);
    }
    
    int write_len = strlen(res->addr+out_count);
    out_count += write_len;
    
    args = cdr(args);
    count++;
  }
  res->size = out_count;
  *((char*)res->addr+out_count) = 0; // terminate string

  return res;
}

Cell* apply_substr(Cell* args, Cell* env) {

  Cell* str = eval(car(args),env);
  if (!str || (str->tag!=TAG_STR && str->tag!=TAG_BYTES)) return alloc_error(ERR_INVALID_PARAM_TYPE);

  args = cdr(args);
  Cell* coffset = eval(car(args),env);
  if (!coffset || coffset->tag!=TAG_INT) return alloc_error(ERR_INVALID_PARAM_TYPE);
  int offset = coffset->value;
  
  int len = str->size;
  
  if (offset<0 || offset>=len) return alloc_error(ERR_OUT_OF_BOUNDS);
  
  args = cdr(args);
  Cell* clen = eval(car(args),env);
  if (clen) {
    if (clen->tag!=TAG_INT) return alloc_error(ERR_INVALID_PARAM_TYPE);
    if (clen->value+offset > str->size) return alloc_error(ERR_OUT_OF_BOUNDS);
    len = clen->value;
  }

  Cell* res = alloc_num_bytes(len+1);
  res->size = len;
  res->tag = str->tag;
  strncpy((char*)res->addr, (char*)str->addr+offset, len);

  *((char*)res->addr+len) = 0; // terminate string

  return res;
}

Cell* apply_list(Cell* args, Cell* env) {
  Cell* res = alloc_nil();
  Cell* a;
  Cell* evd;
  Cell* cur = res;

  while ((a = car(args))) {
    evd = eval(a, env);

    cur->next = alloc_cons(evd, alloc_nil());
    cur = cur->next;
    args = cdr(args);
  }
  cur = cdr(res);
  free(res);
  return cur;
}

Cell* apply_list_append(Cell* args, Cell* env) {
  // TODO: both have to be conses

  Cell* c;
  Cell* new_list = alloc_nil();
  Cell* list_a;
  while ((list_a = car(args))) {
    Cell* evd_list = eval(list_a, env);
    
    while ((c = car(evd_list))) {
      new_list = append(c, new_list);
      evd_list = cdr(evd_list);
    }
    args = cdr(args);
  }
  return new_list;
}

Cell* apply_map(Cell* op, Cell* args, Cell* env) {
  Cell* res = alloc_nil();
  if (!car(args)) return res;
  
  Cell* a;
  Cell* cur = res;

  if (!op) return res;
  
  op = eval(op, env);

  if (op->tag != TAG_LAMBDA && op->tag != TAG_BUILTIN) {
    return res;
  }

  args = car(args);
  args = eval(args, env);
  
  a = args;
  if (!a) return res;

  int i = 0; // idx value to pass to lambda in loop

  if (a->tag == TAG_CONS) {
    // map a list
    a = car(args);
  
    while (a) {
      a = apply(op, alloc_cons(a, alloc_cons(alloc_int(i), 0)), env);
      
      cur->next = alloc_cons(a, alloc_nil());
      args = cdr(args);
      cur = cur->next;
      a = car(args);

      i++;
    }
    
  } else if (a->tag == TAG_STR) {
    // map a string (returns list of substrings)
    // TODO: utf-8

    int i=0;
    for (i=0; i<a->size; i++) {
      Cell* new_str = alloc_num_bytes(2);
      ((char*)new_str->addr)[0] = ((char*)a->addr)[i];
      ((char*)new_str->addr)[1] = 0;
      new_str->tag = TAG_STR;
      
      new_str = apply(op, alloc_cons(new_str, alloc_cons(alloc_int(i), 0)), env);

      cur->next = alloc_cons(new_str,alloc_nil());
      cur = cur->next;
    }
  } else {
    return res;
  }
  
  cur = cdr(res);
  free(res);
  return cur;
}

Cell* apply_filter(Cell* op, Cell* args, Cell* env) {
  Cell* res = alloc_nil();
  Cell* a;
  Cell* evd;
  Cell* cur = res;

  op = eval(op, env);
  args = eval(car(args), env);
  
  while ((a = car(args))) {
    evd = apply(op, alloc_cons(a, 0), env);

    if (evd->value) {
      cur->next = alloc_cons(a, alloc_nil());
      cur = cur->next;
    }
    
    args = cdr(args);
  }
  cur = cdr(res);
  free(res);
  return cur;
}

void debug_cell(Cell* c) {
  printf("cell:  0x%x\r\n",c);
  if (c) {
    if (c->tag == TAG_SYM) {
      printf("sym:   '%s'\r\n",c->addr);
    }
    printf("tag:   %d\r\n",c->tag);
    printf("size:  %d\r\n",c->size);
    printf("addr:  0x%x\r\n",c->addr);
    printf("scope: %d\r\n",c->scope_id);
    printf("flags: 0x%x\r\n",c->flags);
  }
}

Cell* apply_debug(Cell* lambda, Cell* args, Cell* env) {
  printf("debug: %s\r\n",car(args)->addr);
  Cell* arg = eval(car(args), env);
  debug_cell(arg);
  return alloc_nil();
}

Cell* apply(Cell* lambda, Cell* args, Cell* env) {
  //char tmp[256];
  //printf("apply: %s\n",write(lambda,tmp,256));
  if (lambda == 0) {
    return alloc_error(ERR_APPLY_NIL);
  }

  if (lambda->flags & FLAG_FREED) {
    printf("ERROR: applying freed cell %p in %d\n",lambda,active_scope_id);
  }

  // allocs that are returned here are then owned by the scope that they are returned into
  if (lambda->tag == TAG_BUILTIN) {
    switch (lambda->value) {
      case BUILTIN_ADD:
      case BUILTIN_SUB:
      case BUILTIN_MUL:
      case BUILTIN_DIV:
      case BUILTIN_MOD:
        return apply_int_fold(lambda, args, env);
      case BUILTIN_LT:
      case BUILTIN_GT:
      case BUILTIN_EQ:
        return apply_compare(lambda, args, env);

      case BUILTIN_LOAD:
        return apply_load(lambda, args, env);
        
      case BUILTIN_SAVE:
        return apply_save(lambda, args, env);

      case BUILTIN_READ:
        return apply_read(lambda, args, env);

      case BUILTIN_STR:
        return apply_make_str(lambda, args, env);

      case BUILTIN_TYPE: {
        Cell* evd = eval(car(args), env);
        if (!evd) return alloc_int(0); // null
        if (evd->tag == TAG_CONS && evd->addr==0) return alloc_int(0); // nil
        return alloc_int(evd->tag+1);
      }
      
      case BUILTIN_CONCAT:
        return apply_concat(args, env);
        
      case BUILTIN_SUBSTR:
        return apply_substr(args, env);
        
      case BUILTIN_DEBUG:
        return apply_debug(lambda, args, env);

      case BUILTIN_CAR:
        return car(eval(car(args), env));

      case BUILTIN_CDR:
        return cdr(eval(car(args), env));

      case BUILTIN_CONS:
        return alloc_cons(eval(car(args), env), eval(car(cdr(args)), env));

      case BUILTIN_APPEND:
        return apply_list_append(alloc_cons(apply_list_append(args, env), alloc_nil()), env);

      case BUILTIN_REVERSE:
        return apply_list_append(args, env);
                
      case BUILTIN_LEN: {
        Cell* vec = eval(car(args), env);
        if (!vec || (vec->tag!=TAG_BYTES && vec->tag!=TAG_SYM && vec->tag!=TAG_STR)) return alloc_error(ERR_INVALID_PARAM_TYPE);
        return alloc_int(vec->size);
      }

      case BUILTIN_GET: {
        Cell* vec = eval(car(args), env);
        if (!vec || (vec->tag!=TAG_BYTES && vec->tag!=TAG_SYM && vec->tag!=TAG_STR)) return alloc_error(ERR_INVALID_PARAM_TYPE);
        Cell* idx = eval(car(cdr(args)), env);
        if (!idx || idx->tag!=TAG_INT) return alloc_error(ERR_INVALID_PARAM_TYPE);

        if (vec->size <= idx->value || idx->value<0) {
          return alloc_error(ERR_OUT_OF_BOUNDS);
        }
        return alloc_int(((byte*)vec->addr)[idx->value]);
      }

      case BUILTIN_SET: {
        Cell* vec = eval(car(args), env);
        if (!vec || (vec->tag!=TAG_BYTES && vec->tag!=TAG_SYM)) return alloc_error(ERR_INVALID_PARAM_TYPE);
        Cell* idx = eval(car(cdr(args)), env);
        if (!idx || idx->tag!=TAG_INT) return alloc_error(ERR_INVALID_PARAM_TYPE);
        if (vec->size <= idx->value || idx->value<0) {
          return alloc_error(ERR_OUT_OF_BOUNDS);
        }
        Cell* val = eval(car(cdr(cdr(args))), env);
        if (!val || val->tag!=TAG_INT) return alloc_error(ERR_INVALID_PARAM_TYPE);
        byte val_byte = val->value&0xff;
        ((byte*)vec->addr)[idx->value] = val_byte;
        return alloc_int(val_byte);
      }

      case BUILTIN_DEF: {
        // define -> mutates root scope
        Cell* key = car(args);
        Cell* existing = lookup_symbol(key, env);
        //if (is_nil(existing)) {
        Cell* definition = alloc_clone(alloc_cons(key, eval(car(cdr(args)), env)), 0);
        //printf("\r\ndefined %s:\r\n",key->addr);
        //debug_cell(definition->next);
        //printf("\r\n------------------\r\n\r\n");
        
        globals = append(definition, globals);
        return cdr(definition);
        /*} else {
          printf("existing key! %p\n",existing->next);
          Cell* definition = alloc_clone(eval(car(cdr(args)), env), 0);
          free_tree(existing->next); // FIXME: is this 100% safe?
          existing->next = definition;
          return definition;
        }*/
      }
      case BUILTIN_FN: {
        if (car(args)->tag!=TAG_CONS) return alloc_error(ERR_INVALID_PARAM_TYPE);
        if (!car(cdr(args))) return alloc_error(ERR_INVALID_PARAM_TYPE);
        
        Cell* lmb = alloc_lambda(args);
        return lmb;
      }
      case BUILTIN_EVAL: {
        return eval(eval(car(args), env), env);
      }
      case BUILTIN_ALIEN: {
        alien_func f = (alien_func)lambda->next;
        return f(apply_list(args, env), env);
      }
      case BUILTIN_LET: {
        Cell* let = alloc_let(args);
        Cell* res = apply(let, 0, env);
        return res;
      }
      case BUILTIN_IF: {
        Cell* evaled = eval(car(args), env);
        if (evaled && !is_nil(evaled) && evaled->value) {
          return eval(car(cdr(args)), env);
        } else {
          return eval(car(cdr(cdr(args))), env);
        }
      }
      case BUILTIN_LIST: {
        return apply_list(args, env);
      }
      case BUILTIN_MAP: {
        return apply_map(car(args), cdr(args), env);
      }
      case BUILTIN_FILTER: {
        return apply_filter(car(args), cdr(args), env);
      }
      case BUILTIN_QUOTE: {
        return car(args);
      }
      default:
        printf("ERROR: cannot apply unknown op %d.\n",lambda->tag);
        return alloc_error(ERR_UNKNOWN_OP);
    }
  } else if (lambda->tag == TAG_LAMBDA) {
    Cell* arglist = car(lambda);

    Cell* arg_sym = car(arglist);
    Cell* arg_val = car(args);
    // bind env to args
    enter_scope();
    int c = 0;
    while (arg_sym && arg_val) {
      Cell* value = alloc_clone(eval(arg_val, env),active_scope_id);
      //printf("  binding arg %d: %s -> %s\n",c,arg_sym->addr,write(value,tmp,256));
      env = alloc_cons(alloc_cons(arg_sym, value), env); // new pair to be attached to env
      
      arglist = cdr(arglist);
      args = cdr(args);
      
      arg_sym = car(arglist);
      arg_val = car(args);
      c++;
    }
    Cell* result = eval(car(cdr(lambda)), env);
    // free the env conses, but not their content because we borrowed it
    int i = 0;
    for (i=0; i<c; i++) {
      Cell* nextp = cdr(env);
      free(env);
      env = nextp;
    }
    active_scope_id--;
    return result;
  } else if (lambda->tag == TAG_LET) {
    Cell* arglist = car(lambda);
    Cell* arg_val = 0;

    Cell* arg_sym = car(arglist);
    if (arglist && arglist->next) {
      arglist = cdr(arglist);
      arg_val = car(arglist);
    }
    // bind env to args
    enter_scope();
    int c = 0;
    while (arg_sym && arg_val) {
      Cell* value = alloc_clone(eval(arg_val, env),active_scope_id);
      //printf("  binding arg %d: %s -> %s\n",c,arg_sym->addr,write(value,tmp));
      //printf("%d           l %d       l %d\n",active_scope_id, arg_sym->scope_id,value->scope_id);
      env = alloc_cons(alloc_cons(arg_sym, value), env); // new pair to be attached to env
      
      arglist = cdr(arglist);
      arg_sym = car(arglist);
      arglist = cdr(arglist);
      arg_val = car(arglist);
      c++;
    }
    Cell* result = alloc_nil();
    Cell* body = cdr(lambda);
    while (!is_nil(body)) {
      free_tree(result);
      result = eval(car(body), env);
      body = cdr(body);
    }
    int i;
    // free the env conses, but not their content because we borrowed it
    for (i=0; i<c; i++) {
      Cell* nextp = cdr(env);
      free(env);
      env = nextp;
    }
    active_scope_id--;
    return result;
  }
  return lambda;
}

void dump(Cell* expr) {
  char buf[1024];
  lisp_write(expr, buf, 1024);
  printf("<%s>\n",buf);
}

Cell* eval(Cell* expr, Cell* env) {
  eval_depth++;

  //printf("eval %p expr:\n",expr);
  //dump(expr);

  if (eval_depth>MAX_EVAL_DEPTH) {
    eval_depth=0;
    return alloc_error(ERR_MAX_EVAL_DEPTH);
  }

  if (!expr) {
    eval_depth--;
    return 0;
  }

  //printf("expr tag: %d\n",expr->tag);

  if (expr->tag == TAG_INT || expr->tag == TAG_ERROR || expr->tag == TAG_BYTES) {
    eval_depth--;
    return expr;
  } else if (expr->tag == TAG_SYM) {
    Cell* cell = lookup_symbol(expr, env);
    eval_depth--;
    return cell;
  } else if (expr->tag == TAG_CONS) {
    if (is_nil(expr)) {
      eval_depth--;
      return expr;
    }
    Cell* op = car(expr);

    //printf("eval op:\n");
    //dump(op);

    if (!op) {
      printf("error: trying to evaluate null\n");
      exit(0);
    }

    if (op->tag == TAG_SYM) op = eval(op, env);

    if (op && (op->tag == TAG_LAMBDA || op->tag == TAG_BUILTIN)) {
      eval_depth--;
      return apply(op, cdr(expr), env);
    } else {
      //printf("ERROR: cannot apply %p / %d\n\n",op,op->tag);
    }
  } else if (expr->tag == TAG_LAMBDA) {
    eval_depth--;
    return apply(expr, NULL, env);
  }
  eval_depth--;
  return expr;
}

Cell* alloc_cons(Cell* ar, Cell* dr) {
  alloc_cons_count++;
  Cell* cons = malloc(sizeof(Cell));
  cons->scope_id = active_scope_id;
  cons->tag = TAG_CONS;
  cons->addr = ar;
  cons->next = dr;
  cons->flags = 0;
  return cons;
}

Cell* alloc_sym(char* str) {
  alloc_sym_count++;
  Cell* sym = malloc(sizeof(Cell));
  sym->scope_id = active_scope_id;
  sym->tag = TAG_SYM;
  if (str) {
    sym->addr = strdup(str); // TODO: this assumes 0 termination
    sym->size = strlen(str);
  } else {
    sym->addr = 0;
    sym->size = 0;
  }
  sym->next = 0; // could be used for length
  sym->flags = 0;
  return sym;
}

Cell* alloc_int(integer i) {
  alloc_int_count++;
  Cell* num = malloc(sizeof(Cell));
  num->scope_id = active_scope_id;
  num->tag = TAG_INT;
  num->value = i;
  num->next = 0;
  num->flags = 0;
  return num;
}

Cell* alloc_bignum(integer value) {
  Cell* cell = malloc(sizeof(Cell));
  char* tmp = malloc(BIGNUM_INIT_BUFFER_SIZE);
  sprintf(tmp, "%lld", value);
  cell->scope_id = active_scope_id;
  cell->addr = tmp;
  cell->tag = TAG_BIGNUM;
  cell->size = BIGNUM_INIT_BUFFER_SIZE;
  cell->flags = 0;
  return cell;
}

Cell* alloc_num_bytes(integer num_bytes) {
  Cell* cell = malloc(sizeof(Cell));
  cell->addr = malloc(num_bytes);
  memset(cell->addr, 0, num_bytes);
  cell->scope_id = active_scope_id;
  cell->tag = TAG_BYTES;
  cell->size = num_bytes;
  cell->flags = 0;
  return cell;
}

Cell* alloc_bytes() {
  return alloc_num_bytes(SYM_INIT_BUFFER_SIZE);
}

Cell* alloc_string() {
  Cell* cell = malloc(sizeof(Cell));
  cell->addr = malloc(SYM_INIT_BUFFER_SIZE);
  memset(cell->addr, 0, SYM_INIT_BUFFER_SIZE);
  cell->scope_id = active_scope_id;
  cell->tag = TAG_STR;
  cell->size = SYM_INIT_BUFFER_SIZE;
  cell->flags = 0;
  return cell;
}

Cell* alloc_builtin(uint b) {
  alloc_builtin_count++;
  Cell* num = malloc(sizeof(Cell));
  num->scope_id = active_scope_id;
  num->tag = TAG_BUILTIN;
  num->value = b;
  num->next = 0;
  num->flags = 0;
  return num;
}

Cell* alloc_lambda(Cell* def) {
  alloc_lambda_count++;
  Cell* l = malloc(sizeof(Cell));
  l->scope_id = active_scope_id;
  l->tag = TAG_LAMBDA;
  l->addr = car(def); // arguments
  l->next = cdr(def); // body
  l->flags = 0;
  return l;
}

Cell* alloc_let(Cell* def) {
  //alloc_let_count++;
  Cell* l = malloc(sizeof(Cell));
  l->scope_id = active_scope_id;
  l->tag = TAG_LET;
  l->addr = car(def); // bindings
  l->next = cdr(def); // body
  l->flags = 0;
  return l;
}

Cell* alloc_nil() {
  return alloc_cons(0,0);
}

int is_nil(Cell* c) {
  return (!c || (c->addr==0 && c->next==0));
}

Cell* alloc_error(uint code) {
  Cell* c = malloc(sizeof(Cell));
  c->scope_id = active_scope_id;
  c->tag = TAG_ERROR;
  c->value = code;
  c->next = 0;
  return c;
}

Cell* alloc_clone(Cell* orig, uint scope_id) {
  if (!orig) return 0;
  Cell* clone = malloc(sizeof(Cell));
  clone->tag = orig->tag;
  clone->addr = 0;
  clone->next = 0;
  clone->scope_id = scope_id;
  clone->flags = 0;
  clone->size = orig->size;
  if (orig->tag == TAG_SYM || orig->tag == TAG_BIGNUM || orig->tag == TAG_STR) {
    clone->addr = strdup(orig->addr); // FIXME: untracked
  } else if (orig->tag == TAG_BYTES) {
    clone->addr = malloc(orig->size);
    memcpy(clone->addr, orig->addr, orig->size);
  } else if (orig->tag == TAG_CONS || orig->tag == TAG_LAMBDA) {
    if (orig->addr) {
      clone->addr = alloc_clone(orig->addr, scope_id);
    }
    if (orig->next) {
      clone->next = alloc_clone(orig->next, scope_id);
    }
  } else {
    clone->value = orig->value;
  }
  return clone;
}

void free_tree(Cell* root) {
  char buf[128];
  if (root == 0) return;
  //printf("free_tree: %s\n",write(root,buf));

  if (root->scope_id<active_scope_id) {
    //printf("  protected by higher scope (%d>%d).\n",active_scope_id,root->scope_id);
    return;
  }

  if (root->flags == FLAG_FREED) {
    printf("ERROR: double free of cell %p\n",root);
  }
  //printf("freeing: %p @ %d in %d\n",root,root->scope_id,active_scope_id);

  if (root->tag == TAG_CONS) {
    free_cons_count++;
    free_tree((Cell*)root->addr);
    free_tree((Cell*)root->next);
    root->addr=0;
    root->next=0;
  } else if (root->tag == TAG_SYM || root->tag == TAG_BYTES || root->tag == TAG_BIGNUM) {
    free_sym_count++;
    free(root->addr); // frees the string
    root->addr=0;
  } else if (root->tag == TAG_INT) {
    free_int_count++;
  }
  root->flags = FLAG_FREED;
  free(root);
}

#define TMP_BUF_SIZE 4096

char* write_(Cell* cell, char* buffer, int in_list, int bufsize) {

  //printf("writing %p to %p\n",cell,buffer);

  buffer[0]=0;
  if (cell == 0) {
    sprintf(buffer, "null");
  } else if (cell->tag == TAG_INT) {
    snprintf(buffer, bufsize, "%lld", cell->value);
  } else if (cell->tag == TAG_CONS) {
    if (cell->addr == 0 && cell->next == 0) {
      if (!in_list) {
        sprintf(buffer, "nil");
      }
    } else {
      char tmpl[TMP_BUF_SIZE];
      char tmpr[TMP_BUF_SIZE];
      write_((Cell*)cell->addr, tmpl, 0, TMP_BUF_SIZE);

      if (cell->next && ((Cell*)cell->next)->tag==TAG_CONS) {
        write_((Cell*)cell->next, tmpr, 1, TMP_BUF_SIZE);
        if (in_list) {
          if (tmpr[0]) {
            snprintf(buffer, bufsize, "%s %s", tmpl, tmpr);
          } else {
            snprintf(buffer, bufsize, "%s", tmpl); // skip nil at end of list
          }
        } else {
          // we're head of a list
          snprintf(buffer, bufsize, "(%s %s)", tmpl, tmpr);
        }
      } else {
        write_((Cell*)cell->next, tmpr, 0, TMP_BUF_SIZE);
        // improper list
        snprintf(buffer, bufsize, "(%s.%s)", tmpl, tmpr);
      }
    }
  } else if (cell->tag == TAG_SYM) {
    snprintf(buffer, bufsize, "%s", (char*)cell->addr);
  } else if (cell->tag == TAG_STR) {
    snprintf(buffer, min(bufsize,cell->size), "\"%s\"", (char*)cell->addr);
  } else if (cell->tag == TAG_BIGNUM) {
    snprintf(buffer, bufsize, "%s", (char*)cell->addr);
  } else if (cell->tag == TAG_LAMBDA) {
    char tmpr[TMP_BUF_SIZE];
    snprintf(buffer, bufsize, "<proc %s>", write_(cell->next, tmpr, 0, TMP_BUF_SIZE));
  } else if (cell->tag == TAG_BUILTIN) {
    snprintf(buffer, bufsize, "<op %lld>", cell->value);
  } else if (cell->tag == TAG_ERROR) {
    switch (cell->value) {
      case ERR_SYNTAX: sprintf(buffer, "<e0:syntax error.>"); break;
      case ERR_MAX_EVAL_DEPTH: sprintf(buffer, "<e1:deepest level of evaluation reached.>"); break;
      case ERR_UNKNOWN_OP: sprintf(buffer, "<e2:unknown operation.>"); break;
      case ERR_APPLY_NIL: sprintf(buffer, "<e3:cannot apply nil.>"); break;
      case ERR_INVALID_PARAM_TYPE: sprintf(buffer, "<e4:invalid or no parameter given.>"); break;
      case ERR_OUT_OF_BOUNDS: sprintf(buffer, "<e5:out of bounds.>"); break;
      default: sprintf(buffer, "<e%lld:unknown>", cell->value); break;
    }
  } else if (cell->tag == TAG_BYTES) {
    char* hex_buffer = malloc(cell->size*2+1);
    uint i;
    for (i=0; i<cell->size; i++) {
      // FIXME: buffer overflow
      snprintf(hex_buffer+i*2, bufsize-i*2, "%x",((byte*)cell->addr)[i]);
    }
    snprintf(buffer, bufsize, "\[%s]", hex_buffer);
    free(hex_buffer);
  } else {
    snprintf(buffer, bufsize, "<tag:%i>", cell->tag);
  }
  return buffer;
}

char* lisp_write(Cell* cell, char* buffer, int bufsize) {
  return write_(cell, buffer, 0, bufsize);
}

void register_alien_func(char* symname, alien_func func) {
  Cell* builtin = alloc_builtin(BUILTIN_ALIEN);
  builtin->next = func;
  globals = append(alloc_cons(alloc_sym(symname), builtin), globals);
}

void init_lisp() {
  active_scope_id = 0;

  globals = alloc_nil();
  
  globals = append(alloc_cons(alloc_sym("+"),   alloc_builtin(BUILTIN_ADD)), globals);
  globals = append(alloc_cons(alloc_sym("-"),   alloc_builtin(BUILTIN_SUB)), globals);
  globals = append(alloc_cons(alloc_sym("*"),   alloc_builtin(BUILTIN_MUL)), globals);
  globals = append(alloc_cons(alloc_sym("/"),   alloc_builtin(BUILTIN_DIV)), globals);
  globals = append(alloc_cons(alloc_sym("%"),   alloc_builtin(BUILTIN_MOD)), globals);
  globals = append(alloc_cons(alloc_sym("<"),   alloc_builtin(BUILTIN_LT)), globals);
  globals = append(alloc_cons(alloc_sym(">"),   alloc_builtin(BUILTIN_GT)), globals);
  globals = append(alloc_cons(alloc_sym("="),   alloc_builtin(BUILTIN_EQ)), globals);
  globals = append(alloc_cons(alloc_sym("def"), alloc_builtin(BUILTIN_DEF)), globals);
  globals = append(alloc_cons(alloc_sym("fn"),  alloc_builtin(BUILTIN_FN)), globals);
  globals = append(alloc_cons(alloc_sym("if"),  alloc_builtin(BUILTIN_IF)), globals);
  globals = append(alloc_cons(alloc_sym("let"),  alloc_builtin(BUILTIN_LET)), globals);
  globals = append(alloc_cons(alloc_sym("car"), alloc_builtin(BUILTIN_CAR)), globals);
  globals = append(alloc_cons(alloc_sym("cdr"), alloc_builtin(BUILTIN_CDR)), globals);
  globals = append(alloc_cons(alloc_sym("cons"), alloc_builtin(BUILTIN_CONS)), globals);
  globals = append(alloc_cons(alloc_sym("get"),  alloc_builtin(BUILTIN_GET)), globals);
  globals = append(alloc_cons(alloc_sym("set"),  alloc_builtin(BUILTIN_SET)), globals);
  globals = append(alloc_cons(alloc_sym("len"),  alloc_builtin(BUILTIN_LEN)), globals);
  globals = append(alloc_cons(alloc_sym("type"),  alloc_builtin(BUILTIN_TYPE)), globals);
  globals = append(alloc_cons(alloc_sym("list"),  alloc_builtin(BUILTIN_LIST)), globals);
  globals = append(alloc_cons(alloc_sym("load"),  alloc_builtin(BUILTIN_LOAD)), globals);
  globals = append(alloc_cons(alloc_sym("save"),  alloc_builtin(BUILTIN_SAVE)), globals);
  globals = append(alloc_cons(alloc_sym("eval"),  alloc_builtin(BUILTIN_EVAL)), globals);
  globals = append(alloc_cons(alloc_sym("str"),  alloc_builtin(BUILTIN_STR)), globals);
  globals = append(alloc_cons(alloc_sym("debug"),  alloc_builtin(BUILTIN_DEBUG)), globals);
  globals = append(alloc_cons(alloc_sym("read"),  alloc_builtin(BUILTIN_READ)), globals);
  globals = append(alloc_cons(alloc_sym("quote"),  alloc_builtin(BUILTIN_QUOTE)), globals);
  globals = append(alloc_cons(alloc_sym("map"),  alloc_builtin(BUILTIN_MAP)), globals);
  globals = append(alloc_cons(alloc_sym("filter"),  alloc_builtin(BUILTIN_FILTER)), globals);
  globals = append(alloc_cons(alloc_sym("concat"),  alloc_builtin(BUILTIN_CONCAT)), globals);
  globals = append(alloc_cons(alloc_sym("substr"),  alloc_builtin(BUILTIN_SUBSTR)), globals);
  globals = append(alloc_cons(alloc_sym("append"),  alloc_builtin(BUILTIN_APPEND)), globals);
  globals = append(alloc_cons(alloc_sym("reverse"),  alloc_builtin(BUILTIN_REVERSE)), globals);
}
