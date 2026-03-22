/*	$NetBSD$	*/

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void longjmperror(void) __dead;

void
longjmperror(void)
{
	static const char msg[] = "longjmp botch\n";

	(void)write(STDERR_FILENO, msg, sizeof(msg) - 1);
	abort();
	/* NOTREACHED */
}
