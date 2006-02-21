/* vector.c -> a hand-optimized Ruby Vector class */

#include "stdio.h"
#include "math.h"
#include "ruby.h"

#define FALSE 0
#define TRUE 1
#define GET_VEC(orig, p) Data_Get_Struct(orig, Vector, p)

VALUE cVector;

typedef struct {
  double vec[3];
} Vector;

static void vector_free(Vector *p) {
  free(p->vec);
}

static VALUE vector_alloc(VALUE klass) {
  Vector *vector = ALLOC(Vector);
  VALUE obj = Data_Wrap_Struct(klass, 0, vector_free, vector);
  return obj;
}

static VALUE vector_init(int argc, VALUE *argv, VALUE self) {
  Vector *p; GET_VEC(self, p);
  if (argc == 3) {
    p->vec[0] = NUM2DBL(argv[0]);
    p->vec[1] = NUM2DBL(argv[1]);
    p->vec[2] = NUM2DBL(argv[2]);
    return self;
  } else if ((argc < 3) && (argc > 0)) {
    rb_raise(rb_eArgError,
      "ERROR: Either 3 or zero arguments required for Vector.new");
  } else if (argc == 0) {
    p->vec[0] = 0.0;
    p->vec[1] = 0.0;
    p->vec[2] = 0.0;
    return self;
  }
}

static VALUE vector_to_a(VALUE self) {
  Vector *p; GET_VEC(self, p);
  VALUE new_arr = rb_ary_new();
  rb_ary_push(new_arr, rb_float_new(p->vec[0]));
  rb_ary_push(new_arr, rb_float_new(p->vec[1]));
  rb_ary_push(new_arr, rb_float_new(p->vec[2]));
  return new_arr;
}

static VALUE vector_print(VALUE self) {
  Vector *p; GET_VEC(self, p);
  printf("%lf %lf %lf\n", p->vec[0], p->vec[1], p->vec[2]);
  return self;
}

static VALUE vector_mag(VALUE self) {
  Vector *p; GET_VEC(self, p);
  double mag = p->vec[0]*p->vec[0];
  mag += p->vec[1]*p->vec[1];
  mag += p->vec[2]*p->vec[2];
  return rb_float_new(sqrt(mag));
} 

static VALUE vector_entry(VALUE self, VALUE index) {
  Vector *p; GET_VEC(self, p);  
  register int i = NUM2INT(index);
  return rb_float_new(p->vec[i]);
}

static VALUE vector_entry_modify(VALUE self, VALUE index, VALUE new_val) {
  Vector *p; GET_VEC(self, p);
  register int i = NUM2INT(index);
  p->vec[i] = NUM2DBL(new_val);
  return self;
}

static VALUE get_new_vector() {
  VALUE new_vec = 
    rb_funcall(cVector, rb_intern("new"), 0);
  return new_vec;
}

static VALUE vector_add(VALUE self, VALUE other_vec) {
  Vector *p; GET_VEC(self, p);
  Vector *op; GET_VEC(other_vec, op);
  VALUE new_vec = get_new_vector();
  Vector *np; GET_VEC(new_vec, np);
  np->vec[0] = p->vec[0] + op->vec[0];
  np->vec[1] = p->vec[1] + op->vec[1];
  np->vec[2] = p->vec[2] + op->vec[2];
  return new_vec;
}

static VALUE vector_sub(VALUE self, VALUE other_vec) {
  Vector *p; GET_VEC(self, p);
  Vector *op; GET_VEC(other_vec, op);
  VALUE new_vec = get_new_vector();
  Vector *np; GET_VEC(new_vec, np);
  np->vec[0] = p->vec[0] - op->vec[0];
  np->vec[1] = p->vec[1] - op->vec[1];
  np->vec[2] = p->vec[2] - op->vec[2];
  return new_vec;
}

static VALUE vector_inner_product(VALUE self, VALUE other_vec) {
  Vector *p; GET_VEC(self, p);
    
  if ((TYPE(other_vec) == T_FIXNUM) || (TYPE(other_vec) == T_FLOAT)){
    VALUE new_vec = get_new_vector();
    Vector *np; GET_VEC(new_vec, np);
    double factor = NUM2DBL(other_vec);
    np->vec[0] = factor*p->vec[0];
    np->vec[1] = factor*p->vec[1];
    np->vec[2] = factor*p->vec[2];
    return new_vec;
  } else {
    Vector *op; GET_VEC(other_vec, op);
    double inner_prod = p->vec[0]*op->vec[0];
    inner_prod += p->vec[1]*op->vec[1];
    inner_prod += p->vec[2]*op->vec[2];
    return rb_float_new(inner_prod);
  }
}

static VALUE vector_cross_product(VALUE self, VALUE other_vec) {
  Vector *p; GET_VEC(self, p);
  Vector *op; GET_VEC(other_vec, op);
  VALUE new_vec = get_new_vector();
  Vector *np; GET_VEC(new_vec, np);
  
  np->vec[0] = p->vec[1]*op->vec[2] - p->vec[2]*op->vec[1];
  np->vec[1] = p->vec[2]*op->vec[0] - p->vec[0]*op->vec[2];
  np->vec[2] = p->vec[0]*op->vec[1] - p->vec[1]*op->vec[0];
    
  return new_vec;
}

static VALUE vector_abs(VALUE self) {
  Vector *p; GET_VEC(self, p);
  VALUE new_vec = get_new_vector();
  Vector *np; GET_VEC(new_vec, np);
  np->vec[0] = fabs(p->vec[0]);
  np->vec[1] = fabs(p->vec[1]);
  np->vec[2] = fabs(p->vec[2]);
  return new_vec;
}

static VALUE vector_each(VALUE self) {
  Vector *p; GET_VEC(self, p);
  rb_yield(rb_float_new(p->vec[0]));
  rb_yield(rb_float_new(p->vec[1]));
  rb_yield(rb_float_new(p->vec[2]));
  return self;
}

static VALUE vector_each_index(VALUE self) {
  Vector *p; GET_VEC(self, p);
  rb_yield(rb_int_new(0));
  rb_yield(rb_int_new(1));
  rb_yield(rb_int_new(2));
  return self;
}

static VALUE vector_max(VALUE self) {
  Vector *p; GET_VEC(self, p);
  double max = p->vec[0];
  if (p->vec[1] > max)
    max = p->vec[1];
  if (p->vec[2] > max)
    max = p->vec[2];
  return rb_float_new(max);
}

static VALUE vector_min(VALUE self) {
  Vector *p; GET_VEC(self, p);
  double min = p->vec[0];
  if (p->vec[1] < min)
    min = p->vec[1];
  if (p->vec[2] < min)
    min = p->vec[2];
  return rb_float_new(min);
}

static VALUE vector_map(VALUE self) {
  Vector *p; GET_VEC(self, p);
  VALUE new_vec = get_new_vector();
  Vector *np; GET_VEC(new_vec, np);
  np->vec[0] = NUM2DBL(rb_yield( rb_float_new(p->vec[0]) ));
  np->vec[1] = NUM2DBL(rb_yield( rb_float_new(p->vec[1]) ));
  np->vec[2] = NUM2DBL(rb_yield( rb_float_new(p->vec[2]) ));
  return new_vec;
}

static VALUE vector_map_bang(VALUE self) {
  Vector *p; GET_VEC(self, p);
  p->vec[0] = NUM2DBL(rb_yield( rb_float_new(p->vec[0]) ));
  p->vec[1] = NUM2DBL(rb_yield( rb_float_new(p->vec[1]) ));
  p->vec[2] = NUM2DBL(rb_yield( rb_float_new(p->vec[2]) ));
  return self;
}

/* static VALUE vector_pp(VALUE self) {
  Vector *p; GET_VEC(self, p);
  VALUE new_str = rb_str_new2();
  return self;
} */

void Init_vector() {
  cVector = rb_define_class("Vector", rb_cObject);
  rb_define_alloc_func(cVector, vector_alloc);
  rb_define_method(cVector, "initialize", vector_init, -1);
  rb_define_method(cVector, "to_a", vector_to_a, 0);
  rb_define_method(cVector, "mag", vector_mag, 0);
  rb_define_method(cVector, "[]", vector_entry, 1);
  rb_define_method(cVector, "[]=", vector_entry_modify, 2);
  rb_define_method(cVector, "print", vector_print, 0);
  rb_define_method(cVector, "+", vector_add, 1);
  rb_define_method(cVector, "-", vector_sub, 1);
  rb_define_method(cVector, "*", vector_inner_product, 1);
  rb_define_method(cVector, "cross", vector_cross_product, 1);
  rb_define_method(cVector, "abs", vector_abs, 0);
  rb_define_method(cVector, "each", vector_each, 0);
  rb_define_method(cVector, "each_index", vector_each_index, 0);
  rb_define_method(cVector, "max", vector_max, 0);
  rb_define_method(cVector, "min", vector_min, 0);
  rb_define_method(cVector, "map", vector_map, 0);
  rb_define_method(cVector, "map!", vector_map_bang, 0);
  //rb_define_method(cVector, "pp", vector_pp, 0);
}