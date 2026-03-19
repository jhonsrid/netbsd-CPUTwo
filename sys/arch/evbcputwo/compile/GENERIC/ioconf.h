
/* pseudo-devices */
void cpuctlattach(int);
void rndattach(int);
void mdattach(int);
void loopattach(int);
void ptyattach(int);

/* driver structs */
extern struct cfdriver com_cd;
extern struct cfdriver md_cd;
extern struct cfdriver mainbus_cd;
extern struct cfdriver cpu_cd;
