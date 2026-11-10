FROM ubuntu:24.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential git gdb linux-tools-generic python3 time \
    && perf_binary="$(find /usr/lib -path '*/linux-tools*' -type f -name perf | sort | tail -n 1)" \
    && test -n "$perf_binary" \
    && ln -s "$perf_binary" /usr/local/bin/perf \
    && rm -rf /var/lib/apt/lists/*

RUN git config --system --add safe.directory /host

WORKDIR /host
CMD ["/bin/bash"]
