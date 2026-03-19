#include "ioconf.h"
/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "/Users/john/netbsd-CPUTwo/sys/arch/evbcputwo/conf/GENERIC"
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mount.h>

static const struct cfiattrdata mainbuscf_iattrdata = {
	"mainbus", 1, {
		{ "addr", "-1", -1 },
	}
};
static const struct cfiattrdata comcf_iattrdata = {
	"com", 0, {
		{ NULL, NULL, 0 },
	}
};

static const struct cfiattrdata * const com_attrs[] = { &comcf_iattrdata, NULL };
CFDRIVER_DECL(com, DV_TTY, com_attrs);

CFDRIVER_DECL(md, DV_DISK, NULL);

static const struct cfiattrdata * const mainbus_attrs[] = { &mainbuscf_iattrdata, NULL };
CFDRIVER_DECL(mainbus, DV_DULL, mainbus_attrs);

CFDRIVER_DECL(cpu, DV_DULL, NULL);


struct cfdriver * const cfdriver_list_initial[] = {
	&com_cd,
	&md_cd,
	&mainbus_cd,
	&cpu_cd,
	NULL
};

extern struct cfattach mainbus_ca;
extern struct cfattach cpu_ca;
extern struct cfattach com_mainbus_ca;

/* locators */
static int loc[2] = {
	0x3f00000, -1,
};

static const struct cfparent pspec0 = {
	"mainbus", "mainbus", DVUNIT_ANY
};

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

struct cfdata cfdata[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
/*  0: com0 at mainbus? addr 0x3f00000 */
    { "com",		"com_mainbus",	 0, NORM, loc+  0,      0, &pspec0 },
/*  1: mainbus0 at root */
    { "mainbus",	"mainbus",	 0, NORM,    NULL,      0, NULL },
/*  2: cpu0 at mainbus? addr -1 */
    { "cpu",		"cpu",		 0, NORM, loc+  1,      0, &pspec0 },
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};

static struct cfattach * const com_cfattachinit[] = {
	&com_mainbus_ca, NULL
};
static struct cfattach * const mainbus_cfattachinit[] = {
	&mainbus_ca, NULL
};
static struct cfattach * const cpu_cfattachinit[] = {
	&cpu_ca, NULL
};

const struct cfattachinit cfattachinit[] = {
	{ "com", com_cfattachinit },
	{ "mainbus", mainbus_cfattachinit },
	{ "cpu", cpu_cfattachinit },
	{ NULL, NULL }
};

const short cfroots[] = {
	 1 /* mainbus0 */,
	-1
};

/* pseudo-devices */

const struct pdevinit pdevinit[] = {
	{ cpuctlattach, 1 },
	{ rndattach, 1 },
	{ mdattach, 1 },
	{ loopattach, 1 },
	{ ptyattach, 1 },
	{ 0, 0 }
};
