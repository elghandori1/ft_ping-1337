FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    inetutils-ping \
    strace \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ft_ping

COPY . .

CMD ["/bin/bash"]