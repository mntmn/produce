#include "reader.h"

Cell* reader_next_list_cell(Cell* cell, ReaderState* rs) {
  cell->next = alloc_nil();
  cell = cell->next;
  rs->state = PST_ATOM;
  return cell;
}

Cell* reader_end_list(Cell* cell, ReaderState* rs) {
  if (rs->level<1) {
    rs->state = PST_ERR_UNEXP_CLOSING_BRACE;
    return cell;
  }
  rs->level--;
  rs->stack--;
  if (cell->addr) cell->next = alloc_nil();
  cell = *rs->stack;
  Cell* tmpc = cell;

  cell = reader_next_list_cell(cell, rs);
  rs->state = PST_ATOM;
  return cell;
}

ReaderState* read_char(char c, ReaderState* rs) {
  Cell* cell = rs->cell;
  Cell* new_cell;

  if (!cell) {
    // make a root
    cell = alloc_nil();
    cell->next = alloc_nil();
    *rs->stack = cell;
  }

  if (rs->state == PST_ATOM) {
    if (c==' ' || c==13 || c==10) {
      // skip whitespace
    } else if (c>='0' && c<='9') {
      rs->state = PST_NUM;
      new_cell = alloc_int(0);
      new_cell->value = c-'0';
      cell->addr = new_cell;

    } else if (c=='(') {
      // start list
      new_cell = alloc_nil();
      cell->addr = new_cell;
      *rs->stack = cell;

      cell = new_cell;
      rs->stack++;
      rs->level++;
      rs->state = PST_ATOM;
    } else if (c==')') {
      // end of list
      cell = reader_end_list(cell, rs);
    } else if (c=='[') { 
      // bytes (hex notation)
      rs->state = PST_BYTES;
      rs->sym_len = 0;
      new_cell = alloc_bytes();
      cell->addr = new_cell;
    } else if (c=='"') {
      // string
      rs->state = PST_STR;
      rs->sym_len = 0;
      new_cell = alloc_string();
      cell->addr = new_cell;
    } else {
      // symbol
      rs->state = PST_SYM;
      rs->sym_len = 1;
      new_cell = alloc_sym(0);
      new_cell->addr = malloc(SYM_INIT_BUFFER_SIZE);
      memset(new_cell->addr, 0, SYM_INIT_BUFFER_SIZE);
      ((char*)new_cell->addr)[0] = c;
      new_cell->size = SYM_INIT_BUFFER_SIZE; // buffer space
      cell->addr = new_cell;
    }

  } else if (rs->state == PST_NUM || rs->state == PST_NUM_NEG) {
    if (c>='0' && c<='9') {
      // build number
      Cell* vcell = (Cell*)cell->addr;
      if (rs->state == PST_NUM_NEG) {
        vcell->value = vcell->value*10 - (c-'0');
      } else {
        vcell->value = vcell->value*10 + (c-'0');
      }
      if (vcell->value >= BIGNUM_THRESHOLD || vcell->value <= -BIGNUM_THRESHOLD) {
        // convert into bignum (string)
        Cell* new_cell = alloc_bignum(vcell->value);
        free(vcell);
        vcell = new_cell;
        cell->addr = new_cell;

        rs->state = PST_BIGNUM;
        rs->sym_len = strlen(vcell->addr);
      }
    } else if (c==' ' || c==13 || c==10) {
      cell = reader_next_list_cell(cell, rs);
    } else if (c==')') {
      cell = reader_end_list(cell, rs);
    } else {
      rs->state = PST_ERR_UNEXP_JUNK_IN_NUMBER;
    }
  } else if (rs->state == PST_SYM || rs->state == PST_BIGNUM || rs->state == PST_STR) {

    int append = 0;

    if (rs->state == PST_STR) {
      if (c=='"') {
        // string is over
        Cell* vcell = (Cell*)cell->addr;
        vcell->size = (rs->sym_len);
        cell = reader_next_list_cell(cell, rs);
      } else {
        append = 1;
      }
    }
    else {
      if (c==')') {
        cell = reader_end_list(cell, rs);
      } else if (c==' ' || c==13 || c==10) {
        cell = reader_next_list_cell(cell, rs);
      } else if (rs->state == PST_SYM && (c>='0' && c<='9')) {
        Cell* vcell = (Cell*)cell->addr;
        // detect negative number
        if (((char*)vcell->addr)[0] == '-') {
          // we're actually not a symbol, correct the cell.
          rs->state = PST_NUM_NEG;
          vcell->tag = TAG_INT;
          vcell->value = -(c-'0');
        }
      } else {
        append = 1;
      }
    }

    if (append) {
      // build symbol/string/bignum
      Cell* vcell = (Cell*)cell->addr;
      int idx = rs->sym_len;
      rs->sym_len++;
      if (rs->sym_len>=vcell->size-1) {
        // grow buffer
        vcell->addr = realloc(vcell->addr, 2*vcell->size);
        memset(vcell->addr+vcell->size, 0, vcell->size);
        vcell->size = 2*vcell->size;
      }
      ((char*)vcell->addr)[idx] = c;
    }

  } else if (rs->state == PST_BYTES) {
    if (c==']') {
      Cell* vcell = (Cell*)cell->addr;
      vcell->size = (rs->sym_len)/2;
      cell = reader_next_list_cell(cell, rs);
    } else if ((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F')) {
      int n = c;
      if (n>='a') n-=('a'-'9'-1); // hex 'a' to 10 offset
      if (n>='A') n-=('A'-'9'-1); // hex 'a' to 10 offset
      n-='0'; // char to value

      Cell* vcell = (Cell*)cell->addr;
      int idx = rs->sym_len;
      rs->sym_len++;
      if (rs->sym_len>=(vcell->size/2)-1) {
        // grow buffer
        vcell->addr = realloc(vcell->addr, 2*vcell->size); // TODO: check the math
        memset(vcell->addr+vcell->size, 0, vcell->size);
        vcell->size = 2*vcell->size;
      }
      if (idx%2==0) { // even digit
        ((byte*)vcell->addr)[idx/2] = n<<4; // high nybble
      } else { // odd digit
        ((byte*)vcell->addr)[idx/2] |= n;
      }
      
    } else if (c==' ' || c==13 || c==10) {
      // skip
    } else {
      rs->state = PST_ERR_UNEXP_JUNK_IN_BYTES;
    }
  }
  rs->cell = cell;
  return rs;
}

Cell* read_string(char* in) {
  ReaderState rs;
  void* stack_root = malloc(100*sizeof(Cell*));

  rs.state = PST_ATOM;
  rs.cell = 0;
  rs.level = 0;
  rs.stack = stack_root;

  int i=0;
  int len = strlen(in);
  for (i=0; i<len; i++) {
    read_char(in[i], &rs);
    if (rs.state>=10) {
      printf("<read error %d at %d.>\n",rs.state,i);
      break;
    }
    //printf("rs %c: %d\n", in[i], rs.state);
  }
  if (rs.level!=0) {
    printf("<missing %d closing parens.>\n",rs.level);
  }
  if (rs.state!=PST_ATOM) {
    //printf("<read error: unexpected end of input.>\n");
  }

  Cell* root = *rs.stack;
  free(stack_root);

  if (root) {
    Cell* ret = car(root);
    if (root->next) free(root->next);
    free(root);
    return ret;
  }
  return alloc_error(ERR_SYNTAX);
}
