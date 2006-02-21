/* Vector -> A fast vector class for scientific Ruby work */
/* (c) Pradeep Elankumaran, 2005 */

#include "stdio.h"
#include "math.h"
#include "ruby.h"

#define TRUE 1
#define FALSE 0

/* the global variable that contains the Vector class */
VALUE cVector;

typedef struct {
  int size;     /* size of the vector */
  double *vec;  /* the actual vector */
} Vector;

/* allocates a new vector */
static Vector *vector_new() {
  return ALLOC(Vector);
}

/* frees up the memory allocated for the vector in the Vector class */
static void vector_free(Vector *p) {
  free(p->vec);
}

/* allocates the vector, and wraps it up into a data structure that 
ruby can understand */
static VALUE vector_alloc(VALUE klass) {
  Vector *vector;
  VALUE obj;
  vector = vector_new();
  obj = Data_Wrap_Struct(klass, 0, vector_free, vector);
  return obj;
}

/* prints out the vector */
static VALUE vector_print(VALUE self) {
  Vector *p;
  VALUE c = rb_ary_new();
  register int i;
  Data_Get_Struct(self, Vector, p);
  for(i = 0; i < p->size; i++) {
    rb_ary_push(c, rb_float_new(p->vec[i]));
  }
  return c;
}

static VALUE print_size(VALUE self) {
  Vector *p;
  Data_Get_Struct(self, Vector, p);
  return rb_int_new(p->size);
}

/* copies ruby array into a Vector */
static void copy_ary_to_vector(Vector *ptr, VALUE new_vec) {
  register int i;
  /* allocates space for the vector container in the Vector structure */
  ptr->vec = malloc((ptr->size)*sizeof(double));
  for(i = 0; i < (ptr->size); i++) {
    ptr->vec[i] = NUM2DBL(rb_ary_entry(new_vec, i));
  }
}

/* initializes the Vector */
static VALUE vector_init(VALUE self, VALUE vec_vals) {
  Vector *p;
  register int i;
  Data_Get_Struct(self, Vector, p);
  /* gets the size of the ruby array that contains the new values */
  if(NIL_P(vec_vals) != FALSE) {
    p->size = RARRAY(vec_vals)->len;
    copy_ary_to_vector(p, vec_vals); /* loads all the values in */
  } else {
    p->size = 3;
    VALUE zero_arr = rb_ary_new();
    rb_ary_push(zero_arr, rb_int_new(0));
    rb_ary_push(zero_arr, rb_int_new(0));
    rb_ary_push(zero_arr, rb_int_new(0));
    copy_ary_to_vector(p, zero_arr);
  }
  return self;
}

/* changes the vector into another user-specified vector */
static VALUE vector_change(VALUE self, VALUE new_vec) {
  Vector *p;
  register int i;
  Data_Get_Struct(self, Vector, p);
  /* saves the new size */
  p->size = RARRAY(new_vec)->len;
  /* replaces the old values by the new values */
  copy_ary_to_vector(p, new_vec);
  return self;
}

static VALUE return_new_vector(int size) {
  VALUE zero_vec;
  Vector *p;
  register int i;
  zero_vec = rb_funcall(cVector, rb_intern("new"), 1,
                rb_ary_new2(size));
  Data_Get_Struct(zero_vec, Vector, p);
  p->size = size;
  for(i = 0; i < p->size; i++) {
    p->vec = malloc(p->size*sizeof(double)); 
  }
  return zero_vec;
}

static VALUE vector_add(VALUE self, VALUE other_vec) {
  Vector *p, *np, *op;
  VALUE new_vec, flag;
  register int i;
  
  Data_Get_Struct(other_vec, Vector, op);
  Data_Get_Struct(self, Vector, p);
  
  if(op->size == p->size) {
    /* define the new vector */
    new_vec = return_new_vector(op->size);    
    Data_Get_Struct(new_vec, Vector, np);
    np->size = p->size;
    for(i = 0; i < op->size; i++) {
      np->vec[i] = p->vec[i] + op->vec[i];
    }
    return new_vec;
  } else {
    printf("ERROR: vector sizes don't match. Try again!\n");
  }
}

static VALUE vector_subtract(VALUE self, VALUE other_vec) {
  Vector *p, *np, *op;
  VALUE new_vec, flag;
  register int i;
  
  Data_Get_Struct(other_vec, Vector, op);
  Data_Get_Struct(self, Vector, p);
  
  if(op->size == p->size) {
    /* define the new vector */
    new_vec = return_new_vector(op->size);    
    Data_Get_Struct(new_vec, Vector, np);
    np->size = p->size;
    for(i = 0; i < op->size; i++) {
      np->vec[i] = p->vec[i] - op->vec[i];
    }
    return new_vec;
  } else {
    printf("ERROR: vector sizes don't match. Try again!\n");
  }
}

static VALUE vector_inner_product(VALUE self, VALUE other_vec) {
  Vector *p, *op, *np;
  VALUE new_vec;
  int i;
  double ip = 0.0, factor = 0.0;
  
  Data_Get_Struct(self, Vector, p);
  
  if((TYPE(other_vec) == T_FIXNUM) || (TYPE(other_vec) == T_FLOAT)) {  
    new_vec = return_new_vector(op->size);    
    Data_Get_Struct(new_vec, Vector, np);
    np->size = p->size;
    factor = NUM2DBL(other_vec);
    for(i = 0; i < op->size; i++) {
      np->vec[i] = factor*p->vec[i];
    }
    return new_vec;
  } else {
    Data_Get_Struct(other_vec, Vector, op);
    if(op->size == p->size) {
      for(i = 0; i < p->size; i++)
        ip += p->vec[i]*op->vec[i];
      return rb_float_new(ip);
    } else {
      printf("ERROR: vector sizes don't match. Try again!\n");
    }
  }
}

static VALUE vector_magnitude(VALUE self) {
  double mag = sqrt(NUM2DBL(vector_inner_product(self, self)));
  return rb_float_new(mag);
}

static VALUE vector_cross_product(VALUE self, VALUE other_vec) {
  Vector *p, *np, *op;
  VALUE new_vec, flag;
  register int i;
  
  Data_Get_Struct(other_vec, Vector, op);
  Data_Get_Struct(self, Vector, p);
  
  if(op->size == p->size) {
    if(op->size == 3) {
      /* define the new vector */
      new_vec = return_new_vector(op->size);    
      Data_Get_Struct(new_vec, Vector, np);
      np->size = p->size;
      /* cross product */
      np->vec[0] = p->vec[1]*op->vec[2] - p->vec[2]*op->vec[1];
      np->vec[1] = p->vec[2]*op->vec[0] - p->vec[0]*op->vec[2];
      np->vec[2] = p->vec[0]*op->vec[1] - p->vec[1]*op->vec[0];  
      return new_vec;
    } else {
      printf("ERROR: vector's not 3-d. cross product not supported!\n");
    }
  } else {
    printf("ERROR: vector sizes don't match. Try again!\n");
  }
}

static VALUE vector_at_entry(VALUE self, VALUE index) {
  Vector *p;
  register int i = NUM2INT(index);
  Data_Get_Struct(self, Vector, p);
  return rb_float_new(p->vec[i]);
}

static VALUE vector_change_entry(VALUE self, VALUE index, VALUE val) {
  Vector *p;
  register int i = NUM2INT(index);
  Data_Get_Struct(self, Vector, p);
  p->vec[i] = NUM2DBL(val);
  return self;  
}

/* This is a fast vector class for Ruby scientific computation. 
  It is different from NArray because it's not generalized from a matrix.
  These are pure vectors, and as such around 1.3 times faster than NArray 
  on average */
void Init_Vector() {
  cVector = rb_define_class("Vector", rb_cObject);
  rb_define_alloc_func(cVector, vector_alloc);
  rb_define_method(cVector, "initialize", vector_init, -2);
  rb_define_method(cVector, "print", vector_print, 0);
  rb_define_method(cVector, "change", vector_change, 1);
  rb_define_method(cVector, "+", vector_add, 1);
  rb_define_method(cVector, "-", vector_subtract, 1);
  rb_define_method(cVector, "*", vector_inner_product, 1);
  rb_define_method(cVector, "mag", vector_magnitude, 0);
  rb_define_method(cVector, "cross", vector_cross_product, 1);
  rb_define_method(cVector, "[]", vector_at_entry, 1);
  rb_define_method(cVector, "[]=", vector_change_entry,2);
  rb_define_method(cVector, "size", print_size, 0);
}