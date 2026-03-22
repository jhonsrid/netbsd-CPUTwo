cd /Users/john/netbsd-CPUTwo

CC=/Users/john/cputwo-netbsd-toolchain/bin/cputwo--netbsd-clang
AR=/Users/john/cputwo-netbsd-toolchain/bin/cputwo--netbsd-ar
SYSROOT=--sysroot=/Users/john/netbsd-CPUTwo/obj/destdir.evbcputwo
BUILD=/Users/john/netbsd-CPUTwo/ramdisk-build

mkdir -p $BUILD

# --- 1. Create libgcc stubs (if not already present) ---
$CC $SYSROOT -ffreestanding -w -c lib/libc/arch/cputwo/gen/cputwo_sync.c -o /tmp/cputwo_sync.o
$CC $SYSROOT -c lib/libc/arch/cputwo/gen/cputwo_fenv.c -o /tmp/cputwo_fenv.o
$CC $SYSROOT -c lib/libc/arch/cputwo/gen/cputwo_atomic.c -o /tmp/cputwo_atomic.o
for dir in obj/destdir.evbcputwo/lib obj/destdir.evbcputwo/usr/lib; do
mkdir -p $dir
$AR rcs $dir/libgcc.a /tmp/cputwo_sync.o /tmp/cputwo_fenv.o /tmp/cputwo_atomic.o
$AR rcs $dir/libgcc_s.a
$AR rcs $dir/libgcc_eh.a
$AR rcs $dir/libpthread.a
done

# --- 2. Compile minimal userland ---
# init
cat > $BUILD/init.c << 'EOF'
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
int main(void) {
    int fd, status; pid_t pid;
    fd = open("/dev/console", O_RDWR);
    if (fd >= 0) { dup2(fd, 0); dup2(fd, 1); dup2(fd, 2); if (fd > 2) close(fd); }
    write(1, "\nNetBSD/cputwo init started\n", 27);
    for (;;) {
        pid = fork();
        if (pid == 0) {
            char *argv[] = { "sh", NULL };
            char *envp[] = { "HOME=/", "PATH=/bin:/sbin", "TERM=vt100", NULL };
            execve("/bin/sh", argv, envp);
            _exit(1);
        }
        if (pid > 0) waitpid(pid, &status, 0);
    }
}
EOF
$CC $SYSROOT -static -o $BUILD/init $BUILD/init.c

# minimal shell
cat > $BUILD/msh.c << 'EOF'
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define MAXARGS 16
#define BUFSIZE 256
static void prompt(void) { write(1, "# ", 2); }
int main(int argc, char **argv) {
    char buf[BUFSIZE]; char *args[MAXARGS]; int n, i;
    for (;;) {
        prompt();
        n = read(0, buf, BUFSIZE - 1);
        if (n <= 0) break;
        buf[n] = '\0';
        if (n > 0 && buf[n-1] == '\n') buf[n-1] = '\0';
        i = 0; args[i] = strtok(buf, " \t");
        while (args[i] && i < MAXARGS - 1) args[++i] = strtok(NULL, " \t");
        args[i] = NULL;
        if (!args[0] || !args[0][0]) continue;
        if (strcmp(args[0], "cd") == 0) { chdir(args[1] ? args[1] : "/"); continue; }
        if (strcmp(args[0], "exit") == 0) _exit(0);
        pid_t pid = fork();
        if (pid == 0) { execvp(args[0], args); write(2, args[0], strlen(args[0]));
            write(2, ": not found\n", 12); _exit(127); }
        if (pid > 0) { int st; waitpid(pid, &st, 0); }
    }
    return 0;
}
EOF
$CC $SYSROOT -static -o $BUILD/sh $BUILD/msh.c

# ls, cat, echo
$CC $SYSROOT -static -o $BUILD/ls -x c - << 'EOF'
#include <dirent.h>
#include <unistd.h>
#include <string.h>
int main(int c, char **v) { DIR *d=opendir(c>1?v[1]:"."); struct dirent *e;
if(!d){write(2,"ls: error\n",10);return 1;} while((e=readdir(d)))
{write(1,e->d_name,strlen(e->d_name));write(1,"\n",1);} closedir(d); return 0;}
EOF
$CC $SYSROOT -static -o $BUILD/cat -x c - << 'EOF'
#include <unistd.h>
#include <fcntl.h>
int main(int c,char**v){char b[4096];int n,fd;if(c<2){while((n=read(0,b,sizeof b))>0)write(1,b,n);return 0;}
for(int i=1;i<c;i++){fd=open(v[i],O_RDONLY);if(fd<0)continue;while((n=read(fd,b,sizeof
b))>0)write(1,b,n);close(fd);}return 0;}
EOF
$CC $SYSROOT -static -o $BUILD/echo -x c - << 'EOF'
#include <unistd.h>
#include <string.h>
int main(int c,char**v){for(int i=1;i<c;i++){write(1,v[i],strlen(v[i]));write(1,i<c-1?"
":"\n",1);}if(c==1)write(1,"\n",1);return 0;}
EOF

# --- 3. Create ramdisk filesystem ---
ROOTDIR=$BUILD/rootfs
rm -rf $ROOTDIR
mkdir -p $ROOTDIR/{bin,sbin,dev,etc,tmp}
cp $BUILD/init $ROOTDIR/sbin/init
cp $BUILD/sh $ROOTDIR/bin/sh
cp $BUILD/ls $ROOTDIR/bin/ls
cp $BUILD/cat $ROOTDIR/bin/cat
cp $BUILD/echo $ROOTDIR/bin/echo

TOOLDIR=obj/tooldir.Darwin-24.6.0-arm64
$TOOLDIR/bin/nbmakefs -s 2m -t ffs \
    -o bsize=4096,fsize=512,density=8192 \
    $BUILD/ramdisk.img $ROOTDIR

# --- 4. Embed ramdisk into kernel ---
cp sys/arch/evbcputwo/compile/GENERIC/netbsd $BUILD/netbsd
python3 $BUILD/embed_ramdisk.py $BUILD/netbsd $BUILD/ramdisk.img

# --- 5. Boot ---
../qemu-CPUTwo/build-cputwo/qemu-system-cputwo-unsigned -M cputwo-board -kernel $BUILD/netbsd -nographic
