FROM debian:trixie-slim

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      mingw-w64 \
      make \
      libz-mingw-w64 \
      g++-mingw-w64 \
      git && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
