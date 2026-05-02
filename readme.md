# FT Ping Debian


This project provides a Dockerized version of a basic ping utility built on top of a Debian-based image for the test.

## Build And Run the Docker Image

To build the Docker image, use the following command:

```bash
docker build -t ft-ping-debian .
```

To build the Docker image, use the following command:

```bash
docker run -it --cap-add=NET_RAW --rm ft-ping-debian bash

```

⚠️ Crucial: --cap-add=NET_RAW is mandatory. Without it, socket(AF_INET, SOCK_RAW, IPPROTO_ICMP) will fail with Operation not permitted because raw ICMP sockets require elevated privileges.

Quick for test parsing

```bash
docker run -it --rm ft-ping-debian bash
```