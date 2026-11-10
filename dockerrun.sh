#!/bin/sh
set -eu

docker run --rm -it --cap-add SYS_PTRACE --cap-add PERFMON \
    --security-opt seccomp=unconfined \
    -v "$(pwd):/host" -w /host ape-hw3 /bin/bash
