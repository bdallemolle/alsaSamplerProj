typedef struct {
	int (*behavior[MAX_IO])(int, int);			// behaviors for each readable input
	int numBehaviors;
} BEHAVIOR;

BEHAVIOR b;

// initialization function
int initBehavior(CONFIG* c);
