#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>

//for testing purposes, easier to copy and paste into terminal
  //cd cis620/proj4
  //./ldshr_svc

//remember to include ldshr.h header file or Bug
#include "ldshr.h"
#include <rpc/rpc.h> //to use rpc library, e.g. CLIENT type

#define N 5

struct packet {
  void * none;
  CLIENT * cl;
  double result;
  msg *m;
  LinkedList *l;
};


node* newNode(double val) {
  struct node * buf;
  buf = malloc(sizeof(node));
  buf->value = val;
  buf->next = NULL;
  return buf;
}

//pthread function for getload_1--------------------------------
void * getLoad(struct packet *packetbuf) {
  //printf("void * hi\n");
  //double result;
  packetbuf->result = *getload_1(packetbuf->none, packetbuf->cl);
  //result = *getload_1(NULL, packetbuf->cl);
  //printf("getload_1 res: %f\n", result);
  //pthread_exit(&result); //don't need cast to void * //bug: giving seg faults
}

//pthread function for sumqroot_gpu_1----------------------------
void * srgpu(struct packet *packetbuf) {
  packetbuf->result = *sumqroot_gpu_1(packetbuf->m, packetbuf->cl);
}

//pthread function for sumqroot_lst_1----------------------------
void * srlst(struct packet *packetbuf) {
  packetbuf->result = *sumqroot_lst_1(packetbuf->l, packetbuf->cl);
}

//MAIN FUNC******************************************************
int main(int argc, char*argv[]) {
  if (argc != 3 && argc != 6) {
    printf("the number of commandline args is not 3 or 6\n");
    exit(1);
  }
  //printf("client has started\n");
  char *names[N] = {"schubert", "strauss", "mozart", "verdi", "dvorak"};
  double loads[N];
  void * loadsbuf;
  pthread_t tid[N];

  double min1, min2; //lstr1 and lsrt2 holds linked list result
  int i, j, mi1, mi2; //m1 m2 holds index of two lowest load machines
  double dbuf1;
  CLIENT *cl[N];
  FILE *fp;
  //get the loads of the 5 machines
  //printf("client has started\n");
  struct packet *packetbuf[N];
  //packetbuf = malloc(5 * sizeof(struct packetbuf)); //BUG: can't allocate memory here?
  //packetbuf->none = NULL;
  for (i = 0; i < N; i ++) {
    packetbuf[i] = malloc(sizeof(struct packet)); //have to allocate memory
  }

  for (i = 0; i < N; i++) {
    if (!(packetbuf[i]->cl = clnt_create(names[i], RDBPROG, RDBVERS, "tcp"))) {
      clnt_pcreateerror(names[i]);
      exit(1);
    }
    cl[i] = packetbuf[i]->cl;
    packetbuf[i]->none = NULL;
    packetbuf[i]->m = NULL;
    packetbuf[i]->l = NULL;
  }
    //printf("hi\n");
    //packetbuf->cl = cl[i];
  //make threads for getload_1 function calls
  for (i = 0; i < N; i++) {
    //BUG: to get rid of warning, cast void * to getLoad helper function
    if (pthread_create(&tid[i], NULL, (void *) &getLoad, (void *)packetbuf[i]) != 0) {
      printf("pthread_create failed\n");
      exit(1);
    }
  }
    //printf("hi\n");
    //loads[i] = *getload_1((void *)NULL, cl[i]);
  for (i = 0; i < N; i++) {
    pthread_join(tid[i], NULL);
    //printf("hi\n"); //pthread_join is causing a seg fault
    loads[i] = packetbuf[i]->result;
  }
  for (i = 0; i < N; i++) {
    printf("%s: %f ", names[i], loads[i]);
  }
  printf("\n");

  //free(packetbuf);
  //printf("client has started\n");
  //find the 2 machines with lowest loads
  min1 = loads[0];
  mi1 = 0; //BUG: was getting null for mi1 because i forgot to set it
  min2 = 10000000;
  for (i = 1; i < N; i++) {
    if (loads[i] < min1) {
      mi1 = i;
    }
    else if (loads[i] < min2) {
      mi2 = i;
    }
  }
  //free the allocated memory
  for (i = 0; i < N; i++) {
    free(packetbuf[i]);
  }

  //printf("client has started\n");
  //print out which machine will be executed on
  printf("(executed on %s and %s)\n", names[mi1], names[mi2]);

  int n, mean, s1, s2;
  //double gpu1, gpu2; //,lstr1, lstr2; //results of code
  //seg fault at the if comparison
  //char *com = argv[1]; //this causes a bus error
  //printf("argv[1]: %s\n", argv[1]);
//*************************************************************************
  if (strncmp(argv[1], "-gpu", 4) == 0) {
    //printf("client has started\n");
    struct msg * m1;
    struct msg * m2;

    m1 = malloc(sizeof(struct msg));
    m2 = malloc(sizeof(struct msg));

    n = atoi(argv[2]);
    mean = atoi(argv[3]);
    s1 = atoi(argv[4]);
    s2 = atoi(argv[5]);
    //printf("client has started\n");
    m1->n = n - 1;
    m1->mean = mean;
    m1->seed = s1;
    m2->n = n - 1;
    m2->mean = mean;
    m2->seed = s2;
    //printf("hi there\n");
    //call the procedure on the 2 machines
    packetbuf[mi1]->m = m1;
    packetbuf[mi2]->m = m2;

    //gpu1 = *sumqroot_gpu_1(m1, cl[mi1]);
    //gpu2 = *sumqroot_gpu_1(m2, cl[mi2]);
    if ((pthread_create(&tid[mi1], NULL, (void *) srgpu, (void *) packetbuf[mi1])) != 0) {
      printf("error creating thread (sumqroot_gpu)\n");
      exit(1);
    }
    if ((pthread_create(&tid[mi2], NULL, (void *) srgpu, (void *) packetbuf[mi2])) != 0) {
      printf("error creating thread (sumqroot_gpu)\n");
      exit(1);
    }

    pthread_join(tid[mi1], NULL);
    pthread_join(tid[mi2], NULL);

    printf("result for gpu is: %f\n", packetbuf[mi1]->result + packetbuf[mi2]->result);
    free(m1);
    free(m2);
  }
//**************************************************************************
  else if ((strncmp(argv[1], "-lst", 40)) == 0) {
    //struct node *head;
    //struct node *tail;
    if ((fp = fopen(argv[2], "r")) == NULL) {
      printf("error opening file\n");
      exit(1);
    }
    //printf("before linked list creation\n");
    struct LinkedList *lst1;
    struct LinkedList *lst2;
    lst1 = malloc(sizeof (struct LinkedList *));
    lst2 = malloc(sizeof (struct LinkedList *));
    lst1->head = NULL;
    lst1->tail = NULL;
    lst2->head = NULL;
    lst2->tail = NULL;
    struct node *nbuf;
    //nbuf = malloc(sizeof(node));
    int lstflag = 0;
    while ((fscanf(fp, "%lf\n", &dbuf1)) == 1) {
      //printf("%f\n", dbuf1);
      nbuf = newNode(dbuf1);
      lstflag++;
      if (lstflag % 2 == 1) { //lst1-----------------------------------
        if (lst1->head == NULL) { //empty list
          lst1->head = nbuf;
          lst1->head->next = NULL;
          lst1->tail = nbuf;
          lst1->tail->next = NULL;
        }
        else if (lst1->head == lst1->tail) { //one node in lilnked list
          lst1->head->next = nbuf;
          lst1->tail = nbuf;
        }
        else { //more than 1 node in linked list
          lst1->tail->next = nbuf;
          lst1->tail = nbuf;
        }
      }
      else { //lst2----------------------------------------------------
        if (lst2->head == NULL) { //empty list
          lst2->head = nbuf;
          lst2->head->next = NULL;
          lst2->tail = nbuf;
          lst2->tail->next = NULL;
        }
        else if (lst2->head == lst2->tail) { //one node in lilnked list
          lst2->head->next = nbuf;
          lst2->tail = nbuf;
        }
        else { //more than 1 node in linked list
          lst2->tail->next = nbuf;
          lst2->tail = nbuf;
        }
      } //end of else
    } //of while loop
    fclose(fp); //close file here
    //let's make sure the linked list looks right:
    /*
    nbuf = lst1->head;
    while (nbuf != NULL) {
      printf("lst1: %lf\n", nbuf->value);
      nbuf = nbuf->next;
    }
    nbuf = lst2->head;
    while (nbuf != NULL) {
      printf("lst1: %lf\n", nbuf->value);
      nbuf = nbuf->next;
    }
    */
    //printf("before calling sumqroot_lst\n");
    packetbuf[mi1]->l = lst1;
    packetbuf[mi2]->l = lst2;

    if ((pthread_create(&tid[mi1], NULL, (void *) &srlst, (void *)packetbuf[mi1])) != 0) {
      printf("error creating pthread (sumqroot_lst)\n");
      exit(1);
    }

    if ((pthread_create(&tid[mi2], NULL, (void *) &srlst, (void *)packetbuf[mi2])) != 0) {
      printf("error creating pthread (sumqroot_lst)\n");
      exit(1);
    }

    pthread_join(tid[mi1], NULL);
    pthread_join(tid[mi2], NULL);
    //lstr1 = *sumqroot_lst_1(lst1, cl[mi1]);
    //lstr2 = *sumqroot_lst_1(lst2, cl[mi2]);
//BUG: WHY RESULTS CHANGE? Correct result is 7.399662, but can get 7.633031 or 7.166293
    printf("result for linked list cpu is: %f\n", packetbuf[mi1]->result + packetbuf[mi2]->result);
    //free all nodes in linked list
    nbuf = lst1->head;
    while (nbuf != NULL) {
      free(nbuf);
      nbuf = nbuf->next;
    }
    nbuf = lst2->head;
    while (nbuf != NULL) {
      free(nbuf);
      nbuf = nbuf->next;
    }
    //free the 2 linked list memories
    free(lst1);
    free(lst2);
  }
  else { //garbage input
    printf("please type -gpu or -lst as a valid option\n");
    exit(1);
  }
  //printf("hi there\n");
  return 0;
}
