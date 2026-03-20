/*	$NetBSD$	*/

/*
 * CPUTwo kernel stubs for functions not yet fully implemented.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/lwp.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/tty.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/pcb.h>
#include <machine/frame.h>
#include <machine/reg.h>
#include <machine/pmap.h>
#include <machine/db_machdep.h>
#include <machine/userret.h>
#include <ddb/db_variables.h>

/* ------------------------------------------------------------------ */
/*  Console stubs                                                      */
/* ------------------------------------------------------------------ */

/* cn_tab, cons_cdevsw, constty are defined by MI code in kern/cn.c
 * when the console subsystem is properly configured.  For now provide
 * weak definitions so the kernel links.  consinit() in machdep.c
 * sets cn_tab.
 */
struct consdev *cn_tab;
struct tty *volatile constty;

void nullcnpollc(dev_t, int);
void
nullcnpollc(dev_t dev, int on)
{
}

int cngetsn(char *, int);
int
cngetsn(char *buf, int len)
{
	return 0;
}

/* cons_cdevsw: normally from autoconf, stub for now */
const struct cdevsw cons_cdevsw;

/*
 * Console I/O via CPUTwo UART (0x03F00000).
 * These are normally in MI cn.c but that file doesn't exist in this tree.
 */
#define UART_STATUS	(*(volatile uint32_t *)0x03F00000)
#define UART_TX		(*(volatile uint32_t *)0x03F00004)
#define UART_RX		(*(volatile uint32_t *)0x03F00008)
#define UART_TX_READY	0x01
#define UART_RX_AVAIL	0x02

void cnputc(int);
void
cnputc(int c)
{
	while (!(UART_STATUS & UART_TX_READY))
		;
	UART_TX = (uint32_t)c;
}

int cngetc(void);
int
cngetc(void)
{
	while (!(UART_STATUS & UART_RX_AVAIL))
		;
	return (int)UART_RX;
}

void cnpollc(dev_t, int);
void
cnpollc(dev_t dev, int on)
{
}

/* ------------------------------------------------------------------ */
/*  DDB / debugger stubs                                               */
/* ------------------------------------------------------------------ */

int db_active;
db_regs_t ddb_regs;
const struct db_variable db_regs[] = { { NULL, }, };
const struct db_variable * const db_eregs = db_regs;

void cpu_Debugger(void);
void
cpu_Debugger(void)
{
	/* TODO: breakpoint trap */
}

void db_read_bytes(vaddr_t, size_t, char *);
void
db_read_bytes(vaddr_t addr, size_t size, char *data)
{
	const char *src = (const char *)addr;
	while (size--)
		*data++ = *src++;
}

void db_write_bytes(vaddr_t, size_t, const char *);
void
db_write_bytes(vaddr_t addr, size_t size, const char *data)
{
	char *dst = (char *)addr;
	while (size--)
		*dst++ = *data++;
}

void db_stack_trace_print(db_expr_t, bool, db_expr_t, const char *,
    void (*)(const char *, ...));
void
db_stack_trace_print(db_expr_t addr, bool have_addr, db_expr_t count,
    const char *modif, void (*pr)(const char *, ...))
{
	(*pr)("stack trace not implemented\n");
}

void db_set_single_step(db_regs_t *);
void
db_set_single_step(db_regs_t *regs)
{
}

void db_clear_single_step(db_regs_t *);
void
db_clear_single_step(db_regs_t *regs)
{
}

/* ------------------------------------------------------------------ */
/*  mm stubs                                                           */
/* ------------------------------------------------------------------ */

bool mm_md_direct_mapped_phys(paddr_t, vaddr_t *, bool *);
bool
mm_md_direct_mapped_phys(paddr_t pa, vaddr_t *vap, bool *wp)
{
	/* CPUTwo identity-maps all physical memory before MMU is on.
	 * With MMU, kernel maps PA 0 at VA KERNEL_BASE_VIRT.
	 * For now, expose the identity mapping.
	 */
	*vap = (vaddr_t)pa;
	if (wp)
		*wp = false;
	return true;
}

int mm_md_physacc(paddr_t, vm_prot_t);
int
mm_md_physacc(paddr_t pa, vm_prot_t prot)
{
	/* Allow access to all physical addresses */
	return 0;
}

/* ------------------------------------------------------------------ */
/*  pmap stubs                                                         */
/* ------------------------------------------------------------------ */

long pmap_resident_count(struct pmap *);
long
pmap_resident_count(struct pmap *pm)
{
	return 0;
}

long pmap_wired_count(struct pmap *);
long
pmap_wired_count(struct pmap *pm)
{
	return 0;
}

bool
pmap_remove_all(struct pmap *pm)
{
	return false;
}

/* ------------------------------------------------------------------ */
/*  process register access (for ptrace / procfs)                      */
/* ------------------------------------------------------------------ */

int process_read_regs(struct lwp *, struct reg *);
int
process_read_regs(struct lwp *l, struct reg *regs)
{
	struct trapframe *tf = l->l_md.md_utf;
	if (tf == NULL)
		return EIO;
	memcpy(regs->r, tf->tf_r, sizeof(regs->r));
	regs->r_pc = tf->tf_pc;
	return 0;
}

int process_write_regs(struct lwp *, const struct reg *);
int
process_write_regs(struct lwp *l, const struct reg *regs)
{
	struct trapframe *tf = l->l_md.md_utf;
	if (tf == NULL)
		return EIO;
	memcpy(tf->tf_r, regs->r, sizeof(regs->r));
	tf->tf_pc = regs->r_pc;
	return 0;
}

int process_set_pc(struct lwp *, void *);
int
process_set_pc(struct lwp *l, void *addr)
{
	struct trapframe *tf = l->l_md.md_utf;
	if (tf == NULL)
		return EIO;
	tf->tf_pc = (uint32_t)(uintptr_t)addr;
	return 0;
}

int cpu_mcontext_validate(struct lwp *, const mcontext_t *);
int
cpu_mcontext_validate(struct lwp *l, const mcontext_t *mcp)
{
	return 0;
}

int procfs_getcpuinfstr(char *, size_t *);
int
procfs_getcpuinfstr(char *buf, size_t *len)
{
	*len = 0;
	return 0;
}

/* ------------------------------------------------------------------ */
/*  Misc arch stubs                                                    */
/* ------------------------------------------------------------------ */

#include <sys/exec.h>
#include <sys/syscallargs.h>

void cnflush(void);
void
cnflush(void)
{
}

void cpu_idle(void);
void
cpu_idle(void)
{
	/* halt until next interrupt */
}

void md_child_return(struct lwp *);
void
md_child_return(struct lwp *l)
{
	struct trapframe *tf = l->l_md.md_utf;
	tf->tf_r[0] = 0;	/* return 0 */
	tf->tf_r[1] = 0;	/* no error */
}

void cpu_spawn_return(struct lwp *);
void
cpu_spawn_return(struct lwp *l)
{
	md_child_return(l);
}

void startlwp(void *);
void
startlwp(void *arg)
{
	userret(curlwp);
}

void
setregs(struct lwp *l, struct exec_package *pack, vaddr_t stack)
{
	struct trapframe *tf = l->l_md.md_utf;
	memset(tf, 0, sizeof(*tf));
	tf->tf_pc = pack->ep_entry;
	tf->tf_r[13] = stack;	/* SP */
}

int cpu_coredump(struct lwp *, struct coredump_iostate *, struct core *);
int
cpu_coredump(struct lwp *l, struct coredump_iostate *iocookie,
    struct core *chdr)
{
	return 0;
}

int sys_sysarch(struct lwp *, const struct sys_sysarch_args *, register_t *);
int
sys_sysarch(struct lwp *l, const struct sys_sysarch_args *uap,
    register_t *retval)
{
	return ENOSYS;
}

int
vmapbuf(struct buf *bp, vsize_t len)
{
	return 0;
}

void
vunmapbuf(struct buf *bp, vsize_t len)
{
}

db_addr_t db_disasm(db_addr_t, bool);
db_addr_t
db_disasm(db_addr_t loc, bool altfmt)
{
	return loc + 4;
}
