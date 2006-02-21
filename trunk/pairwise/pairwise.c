#include "stdio.h"
#include "ruby.h"
#include "math.h"

#define GET_VEC(val,p) Data_Get_Struct(val, Vector, p)
typedef struct {
  double vec[3];
} Vector;

static VALUE get_new_vector() {
  return rb_eval_string("Vector.new");
}

static VALUE pairwise_acc( VALUE self, VALUE b, VALUE other_body, 
                           VALUE other_mass, VALUE l_eps) {
  Vector *p; GET_VEC(b, p);
  Vector *op; GET_VEC(other_body, op);
  double eps = NUM2DBL(l_eps);
  double mass = NUM2DBL(other_mass);
  VALUE new_vec = get_new_vector();
  Vector *np; GET_VEC(new_vec, np);
  
  double p0 = op->vec[0] - p->vec[0];
  double p1 = op->vec[1] - p->vec[1];
  double p2 = op->vec[2] - p->vec[2];
  double rsqr = p0*p0 + p1*p1 + p2*p2 + eps*eps;
  double r3 = sqrt(rsqr)*rsqr;
  
  np->vec[0] = p0*(mass/r3);
  np->vec[1] = p1*(mass/r3);
  np->vec[2] = p2*(mass/r3);
  return new_vec;
}

static VALUE pairwise_potential(VALUE self, VALUE mass, VALUE other_mass,
  VALUE pos, VALUE other_pos, VALUE eps) {
  Vector *p; GET_VEC(pos, p);
  Vector *op; GET_VEC(other_pos, p);
  double fmass = NUM2DBL(mass);
  double fother_mass = NUM2DBL(other_mass);
  double feps = NUM2DBL(eps);
  double epot = 0.0;
  
  double r0 = op->vec[0] - p->vec[0];
  double r1 = op->vec[1] - p->vec[1];
  double r2 = op->vec[2] - p->vec[2];
  
  epot = (-fmass*fother_mass)/sqrt(r0*r0 + r1*r1 + r2*r2 + eps*eps);
  return rb_float_new(epot);
}



VALUE mSpeedUp;
void Init_pairwise() {
  mSpeedUp = rb_define_module("SpeedUp");
  rb_define_module_function(mSpeedUp, "pairwise_acc", pairwise_acc, 4);
  rb_define_module_function(mSpeedUp, "get_potential_energy",
                            pairwise_potential, 5);
}