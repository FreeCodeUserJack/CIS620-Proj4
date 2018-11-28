
struct node {
  double value;
  struct node *next;
};

struct LinkedList {
  struct node *head;
  struct node *tail;
};

struct msg {
  int n;
  int mean;
  int seed;
};

program RDBPROG { /* could manage multiple servers */
	version RDBVERS {
		double GETLOAD() = 1;
    /* Bug: must pass parameters as a struct, also can't have // for comment */
		double SUMQROOT_GPU(msg) = 2;
		double SUMQROOT_LST(LinkedList) = 3;
	} = 1;
} = 0x22233344;
