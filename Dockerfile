FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      autoconf \
      automake \
      libtool \
      pkg-config \
      make \
      python3 \
      ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# Toolchain is expected to be mounted at /opt/mini from the host.
# Example: -v /path/to/mini_toolchain-v1.0/mini:/opt/mini

WORKDIR /work
