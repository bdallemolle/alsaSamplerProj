typedef struct {
	int (*event[MAX_IO])(int);
	int args[MAX_IO];
} EVENT;

EVENT e;

// initialization function
int initEvents(CONFIG* c);