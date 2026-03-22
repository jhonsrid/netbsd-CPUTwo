#!/bin/bash

set -euo pipefail

rm -rf /Users/john/netbsd-CPUTwo/obj/

OPTS="-m evbcputwo -U -V EXTERNAL_TOOLCHAIN=/Users/john/cputwo-netbsd-toolchain -V HAVE_LLVM=yes -V MKGCC=no -V MKLLVM=no -V MKGROFF=no -V MKUNWIND=no -V MKCXX=no  -V HAVE_LIBGCC_EH=yes -V MKSSP=no -V USE_SSP=no -V MKPROFILE=no -V MKPROFILE=no -V MKPIC=no -V MKCRYPTO=no -j 24"

./build.sh ${OPTS} tools
read -p "Tools built, press to continue"

./build.sh ${OPTS} kernel=GENERIC
read -p "Kernel built, press to continue"

# -u REQUIRED
./build.sh ${OPTS} -u distribution

