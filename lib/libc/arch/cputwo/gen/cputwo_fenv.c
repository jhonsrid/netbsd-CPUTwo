/*	$NetBSD$	*/

/*
 * CPUTwo floating-point environment stubs.
 * No FPU — always round to nearest, no exceptions.
 */

#include <sys/types.h>
#include <machine/fenv.h>

const fenv_t __fe_dfl_env = 0;

int fegetround(void) { return FE_TONEAREST; }
int fesetround(int r) { return (r == FE_TONEAREST) ? 0 : -1; }
int fegetenv(fenv_t *e) { *e = __fe_dfl_env; return 0; }
int fesetenv(const fenv_t *e) { return 0; }
int feholdexcept(fenv_t *e) { *e = __fe_dfl_env; return 0; }
int feupdateenv(const fenv_t *e) { return 0; }
int fetestexcept(int ex) { return 0; }
int feraiseexcept(int ex) { return 0; }
int feclearexcept(int ex) { return 0; }
int fegetexceptflag(fexcept_t *f, int ex) { *f = 0; return 0; }
int fesetexceptflag(const fexcept_t *f, int ex) { return 0; }
int feenableexcept(int ex) { return 0; }
int fedisableexcept(int ex) { return 0; }
int fegetexcept(void) { return 0; }
