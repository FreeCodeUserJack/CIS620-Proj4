#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h> //include this or bug
#include "ldshr.h" //BUG: types are undefined if don't include header file
//bug: use quotation marks for rdb.h because file is in dir
#include <math.h>
#include <pthread.h>

double reduction(int, int, int); //C++ function

//no main func in server code
//bug: had to had void * none to getload parameter or else type doesn't match
double * getload_1_svc(void * none, struct svc_req *rqp) {
  double * result = malloc(sizeof(double *));
  double ret[3];
  getloadavg(ret, 3);
  *result = ret[0];
  //printf("%f\n", *result);
  return result;
}

//##########################################################################
double * sumqroot_gpu_1_svc(msg *m, struct svc_req *rqp) {
  double *result = malloc(sizeof(double *));
  //int n, mean, seed;
  printf("sumqroot_gpu: %d %d %d\n", m->n, m->mean, m->seed);
  *result = reduction(m->n, m->mean, m->seed);
  return result;
}

//##########################################################################
//helper functions + map and reduce functions, sumqroot_lst at the bottom

double qroot(double val) {
  return sqrt(sqrt(val));
}

double sum(double v1, double v2) {
  return v1 + v2;
}

void map(double (*f)(double), LinkedList *l) {
  struct node * nodebuf;
  nodebuf = l->head;
  while (nodebuf != NULL) {
    nodebuf->value = (*f)(nodebuf->value);
    nodebuf = nodebuf->next;
  }
}

double reduce(double (*f)(double, double), LinkedList *l) {
  struct node * nodebuf;
  double result = 0;
  nodebuf = l->head;
  while (nodebuf != NULL) {
    result =  (*f)(result, nodebuf->value);
    nodebuf = nodebuf->next;
  }
  return result;
}

double * sumqroot_lst_1_svc(LinkedList *l, struct svc_req *rqp) {
  //printf("sumqroot_lst\n");
  double * result = malloc(sizeof(double *));
  map(&qroot, l);
  *result = reduce(&sum, l);
  return result;
}
