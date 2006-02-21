/* tree.c -> A hand-optimized, simple Barnes & Hut Tree extension 
             for Ruby work
   author: Pradeep Elankumaran, 2006
   based on Piet Hut & Jun Makino's Ruby treecode contained in the
   Maya (http://www.artcompsci.org) distribution */

#include "stdio.h"
#include "math.h"
#include "ruby.h"
#include "assert.h"

#define TRUE 1
#define FALSE 0
#define GET_NODE(val, p) Data_Get_Struct(val, TreeNode, p)
#define GET_VEC(val, p) Data_Get_Struct(val, Vector, p)

VALUE cTreeNode;

typedef struct {
  VALUE center;
  double size;
  double mass;
  VALUE pos;
  VALUE children[8];
} TreeNode;

typedef struct {
  double vec[3];
} Vector;

/* ALLOCATION METHODS ----------------------- */
static void node_free(TreeNode *p) {
  free(p);
}

static VALUE node_alloc(VALUE klass) {
  TreeNode *tn = ALLOC(TreeNode);
  VALUE obj = Data_Wrap_Struct(klass, 0, node_free, tn);
  return obj; 
}

static VALUE node_initialize(VALUE self, VALUE rb_center, VALUE rb_size) {
  TreeNode *t; GET_NODE(self, t);
  Vector *p; GET_VEC(rb_center, p);
  register int i;
  
  t->center = rb_center;
  t->pos = rb_eval_string("Vector.new");
  t->size = NUM2DBL(rb_size);
  t->mass = 0.0;
  for(i = 8; i--; ) {
    t->children[i] = Qnil;
  }
  return self;
}

/* UTILITY METHODS ---------------------------------- */

int get_octant(VALUE pos, VALUE center) {
  Vector *p; GET_VEC(pos, p);
  Vector *c; GET_VEC(center, c);
  register int octant = 0, i = 0;
  for(i = 0; i < 3; i++) {
    octant *= 2;
    if (p->vec[i] > c->vec[i])
      octant += 1;
  }
  return octant;
}

VALUE get_offset(int corner) {
  VALUE offset_vec = rb_eval_string("Vector.new");
  Vector *p; GET_VEC(offset_vec, p);
  
  p->vec[2] = (corner & 1)*2 - 1;
  corner >>=1;
  p->vec[1] = (corner & 1)*2 - 1;
  corner >>=1;
  p->vec[0] = (corner & 1)*2 - 1;
  return offset_vec;
}

int check_classname(char *str1, char *str2) {
  if (strcmp(str1, str2) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static VALUE node_print(VALUE self) {
  TreeNode *t; GET_NODE(self, t);
  Vector *c; GET_VEC(t->center, c);
  Vector *p; GET_VEC(t->pos, p);
  rb_p(self);
  printf("---------\n");
  printf("size: %g\n", t->size);
  printf("center: %g %g %g\n", c->vec[0], c->vec[1], c->vec[2]);
  printf("pos: %g %g %g\n", p->vec[0], p->vec[1], p->vec[2]);
  printf("mass: %g\n", t->mass);
  printf("----------\n");
  return self;
}


/* TREE METHODS ------------------------------------ */

/* loads particles into the tree */
static VALUE node_loadtree(VALUE self, VALUE body) {
  TreeNode *t; GET_NODE(self, t);
  int corner = get_octant(rb_iv_get(body, "@pos"), t->center);
  if (NIL_P(t->children[corner])) {
    t->children[corner] = body;
    return self;
  }
  if (strcmp(rb_obj_classname(t->children[corner]), "Body") == 0) {   
    /* back-up body at the location */
    VALUE tmp_body = t->children[corner];
    double child_size = t->size / 2.0;
    
    /* get the center offset for the new node */
    Vector *c; GET_VEC(t->center, c);
    VALUE offset_vec = get_offset(corner);
    Vector *o; GET_VEC(offset_vec, o);
    
    /* create the center vector for the new node */
    VALUE new_vec = rb_eval_string("Vector.new");
    Vector *p; GET_VEC(new_vec, p);
    p->vec[0] = c->vec[0] + child_size*o->vec[0];
    p->vec[1] = c->vec[1] + child_size*o->vec[1];
    p->vec[2] = c->vec[2] + child_size*o->vec[2];
    
    /* create a new node and replace the old body with it */
    t->children[corner] = 
      rb_funcall(cTreeNode, rb_intern("new"), 2, 
                 new_vec, rb_float_new(child_size));
    
    /* load the two bodies into the new node */
    rb_funcall(t->children[corner], rb_intern("load_tree"), 1, tmp_body);
    rb_funcall(t->children[corner], rb_intern("load_tree"), 1, body);
    return self;
  }
  return self;
}

/* computes the center of mass of the node */
static VALUE node_center_of_mass(VALUE self) {
  TreeNode *t; GET_NODE(self, t);
  TreeNode *o;
  Vector *p; GET_VEC(t->pos, p);
  Vector *op;
  register int i;
  double omass;
  t->mass = 0.0;
  
  for(i = 0; i < 8; i++) {
    if (check_classname(rb_obj_classname(t->children[i]), "TreeNode")) {
      rb_funcall(t->children[i], rb_intern("center_of_mass"), 0);
      omass = NUM2DBL(rb_funcall(t->children[i], rb_intern("mass"), 0));
      GET_NODE(t->children[i], o);
      GET_VEC(o->pos, op);
      t->mass += omass;
      p->vec[0] += omass * op->vec[0];
      p->vec[1] += omass * op->vec[1];
      p->vec[2] += omass * op->vec[2];
    } else if
      (check_classname(rb_obj_classname(t->children[i]), "Body")) {
      omass = NUM2DBL(rb_funcall(t->children[i], rb_intern("mass"),0));
      VALUE new_pos = rb_funcall(t->children[i], rb_intern("pos"), 0);
      GET_VEC(new_pos, op);
      t->mass += omass;
      p->vec[0] += omass * op->vec[0];
      p->vec[1] += omass * op->vec[1];
      p->vec[2] += omass * op->vec[2];
    }
  }
  p->vec[0] /= t->mass;
  p->vec[1] /= t->mass;
  p->vec[2] /= t->mass;
  rb_iv_set(self, "@pos", t->pos);
  rb_iv_set(self, "@mass", rb_float_new(t->mass));
  rb_iv_set(self, "@size", rb_float_new(t->size));
  return self;
}

static VALUE self_get_other_acc(VALUE self, VALUE other, VALUE epsilon) {
  TreeNode *t; GET_NODE(self, t);
  Vector *p; GET_VEC(t->pos, p);
  Vector *op;
  VALUE rji = rb_eval_string("Vector.new");
  Vector *rjip; GET_VEC(rji, rjip);
  double eps = NUM2DBL(epsilon);
  int nil_flag = TRUE;
  
  if(check_classname(rb_obj_classname(other), "Body")) {
    GET_VEC(rb_iv_get(other, "@pos"), op);
    nil_flag = FALSE;
  } else if (check_classname(rb_obj_classname(other), "TreeNode")) {
    TreeNode *o; GET_NODE(other, o);
    GET_VEC(o->pos, op);
    nil_flag = FALSE;
  }
 
  if (self == other) {  
    return rb_eval_string("Vector.new");
  } else {
    if (nil_flag == FALSE) {
      rjip->vec[0] = p->vec[0] - op->vec[0];
      rjip->vec[1] = p->vec[1] - op->vec[1];
      rjip->vec[2] = p->vec[2] - op->vec[2];
      double r2 = eps*eps + rjip->vec[0]*rjip->vec[0] + 
                  rjip->vec[1]*rjip->vec[1] + rjip->vec[2]*rjip->vec[2];
      rjip->vec[0] *= t->mass/(r2*sqrt(r2));
      rjip->vec[1] *= t->mass/(r2*sqrt(r2));
      rjip->vec[2] *= t->mass/(r2*sqrt(r2));
      return rji;
    }
  }
}

static VALUE node_get_acc(VALUE self, VALUE body, 
                          VALUE tolerance, VALUE epsilon) {
  TreeNode *t; GET_NODE(self, t);
  Vector *p; GET_VEC(t->pos, p);
  VALUE body_pos = rb_iv_get(body, "@pos");
  Vector *bp; GET_VEC(body_pos, bp);
  double tol = NUM2DBL(tolerance);
  VALUE acc = rb_eval_string("Vector.new");
  Vector *a; GET_VEC(acc, a);
  
  Vector dist;
  dist.vec[0] = bp->vec[0] - p->vec[0];
  dist.vec[1] = bp->vec[1] - p->vec[1];
  dist.vec[2] = bp->vec[2] - p->vec[2];
  double dist_mag = sqrt(dist.vec[0]*dist.vec[0] + 
                         dist.vec[1]*dist.vec[1] + 
                         dist.vec[2]*dist.vec[2]);
  
  if ( (2*t->size) > (tol*dist_mag) ) {
    register int i;
    for(i = 0; i < 8; i++ ) {
      if (NIL_P(t->children[i]) == FALSE) {
        VALUE c_acc = rb_funcall(t->children[i], rb_intern("get_acc"), 3,
          body, tolerance, epsilon);
        Vector *cap; GET_VEC(c_acc, cap);
        a->vec[0] += cap->vec[0];
        a->vec[1] += cap->vec[1];
        a->vec[2] += cap->vec[2];
        return acc;
      }
    }
  } else {
    printf("other!\n");
    self_get_other_acc(self, body, epsilon);
  }
}


/* ACCESSOR METHODS -------------------------------------------*/
static VALUE node_mass (VALUE self) {
  TreeNode *t; GET_NODE(self, t);
  return rb_float_new(t->mass);
}

static VALUE node_pos (VALUE self) {
  TreeNode *t; GET_NODE(self, t);
  return t->pos;
}

static VALUE node_size (VALUE self) {
  TreeNode *t; GET_NODE(self, t);
  return rb_float_new(t->size);
}

static VALUE node_child_each(VALUE self) {
  TreeNode *t; GET_NODE(self, t);
  register int i;
  for(i = 0; i < 8; i++) {
    rb_yield(t->children[i]);
  }
  return self;
}


/* MAIN RUBY DECLARATION ------------------------------------- */
void Init_tree() {
  rb_require("vector/vector");
  rb_require("pairwise/pairwise");
  cTreeNode = rb_define_class("TreeNode", rb_cObject);
  rb_define_alloc_func(cTreeNode, node_alloc);
  rb_define_method(cTreeNode, "initialize", node_initialize, 2);
  rb_define_method(cTreeNode, "load_tree", node_loadtree, 1);
  rb_define_method(cTreeNode, "print", node_print, 0);
  rb_define_method(cTreeNode, "center_of_mass", node_center_of_mass, 0);
  //rb_define_method(cTreeNode, "get_acc", node_get_acc, 3);
  rb_define_method(cTreeNode, "mass", node_mass, 0);
  rb_define_method(cTreeNode, "pos", node_pos, 0);
  rb_define_method(cTreeNode, "size", node_size, 0);
  rb_define_method(cTreeNode, "each_child", node_child_each, 0);
}