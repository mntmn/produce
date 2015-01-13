#include "minilisp.h"
#include "bignum.h"

// note that these operations store the result in Cell a.

Cell* bignum_add(Cell* a, Cell* z) {
  Cell* b = z;
  if (z->tag == TAG_INT) { // coerce z into bignum
    b = alloc_bignum(z->value); // TODO leak
  }
  uint a_len = strlen(a->addr);
  uint b_len = strlen(b->addr);

  if (((char*)a->addr)[0] == '-' && ((char*)b->addr)[0] != '-') {
    // -a + b --> b-a
    ((char*)a->addr)[0]='0';
    Cell* tmp = alloc_clone(b,b->scope_id);
    bignum_sub(tmp, a);
    free(a->addr);
    a->addr = tmp->addr; a->size = tmp->size;
    free(tmp);
    return a;
  }

  if (((char*)a->addr)[0] != '-' && ((char*)b->addr)[0] == '-') {
    // a + -b --> a-b
    Cell* tmp = alloc_clone(b,b->scope_id);
    ((char*)tmp->addr)[0]='0';
    a = bignum_sub(a,tmp);
    free_tree(tmp);
    return a;
  }

  uint len = (a_len>b_len)?a_len:b_len;
  char* res_bignum = malloc(len+3);
  memset(res_bignum, 0, len+3);

  uint carry = 0;
  int ai = a_len-1;
  int bi = b_len-1;
  int ri;
  for (ri=len-1; ri>=0; ri--) {
    uint digit_a = 0;
    if (ai>=0) digit_a = ((char*)a->addr)[ai--]-'0';
    uint digit_b = 0;
    if (bi>=0) digit_b = ((char*)b->addr)[bi--]-'0';

    uint res_digit = digit_a + digit_b + carry;

    if (res_digit>9) {
      carry = 1;
      res_digit-=10;
    } else {
      carry = 0;
    }

    res_bignum[ri] = (char)res_digit+'0';
  }
  if (carry) {
    memcpy(res_bignum+1,res_bignum,len+1);
    res_bignum[0]='1';
  }

  free(a->addr);
  a->addr = res_bignum;
  a->size = len+2;
  return a;
}

Cell* bignum_sub(Cell* a, Cell* z) {
  Cell* b = z;
  if (z->tag == TAG_INT) { // coerce z into bignum
    b = alloc_bignum(z->value); // TODO leak
  }
  uint a_len = strlen(a->addr);
  uint b_len = strlen(b->addr);

  if (((char*)b->addr)[0] == '-') {
    // a - -b    -a - -b
    // -> a + b  -> -a + b
    Cell* tmp = alloc_clone(b, b->scope_id);
    ((char*)tmp->addr)[0] = '0';
    a = bignum_add(a,tmp);
    free_tree(tmp);
    return a;
  }

  if (((char*)a->addr)[0] == '-') {
    // -a - b   -> -(a + b)
    ((char*)a->addr)[0] = '0';
    a = bignum_add(a,b);
    memcpy(a->addr+1,a->addr,strlen(a->addr)+1);
    ((char*)a->addr)[0] = '-';
    return a;
  }

  uint len = (a_len>b_len)?a_len:b_len;
  char* res_bignum = malloc(len+3);
  memset(res_bignum, 0, len+3);

  uint carry = 0;
  int ai = a_len-1;
  int bi = b_len-1;
  int ri;
  for (ri=len-1; ri>=0; ri--) {
    uint digit_a = 0;
    if (ai>=0) digit_a = ((char*)a->addr)[ai--]-'0';
    uint digit_b = 0;
    if (bi>=0) digit_b = ((char*)b->addr)[bi--]-'0';

    int res_digit = 0;

    digit_b+=carry;
    if (digit_b>digit_a) {
      carry = 1;
      if (ai<0) {
        res_digit = digit_b;
      } else {
        res_digit = 10+digit_a-digit_b;
      }
    } else {
      carry = 0;
      res_digit = digit_a-digit_b;
    }

    res_bignum[ri] = (char)res_digit+'0';
  }
  if (carry) {
    // negative? start over by swapping and flagging with minus
    // TODO: can we do this more efficiently?
    Cell* tmp = bignum_sub(alloc_clone(z, z->scope_id), a); // TODO: memory handling correct?

    free(a->addr);
    a->addr = tmp->addr;
    a->size = tmp->size;
    free(tmp);
    memcpy(a->addr+1,a->addr,strlen(a->addr)+1);
    ((char*)a->addr)[0] = '-';
    return a;
  }
  // skip leading zeroes
  int i;
  int skip = 0;
  int stop = 0;
  for (i=0; i<len; i++) {
    res_bignum[i-skip]=res_bignum[i];
    if (!stop) {
      if (res_bignum[i]=='0') skip++;
      else stop = 1;
    }
  }
  res_bignum[len-skip]=0;
  //printf("snip at %d: %s\n",len-skip,res_bignum);

  free(a->addr);
  a->addr = res_bignum;
  a->size = len+3;

  return a;
}
