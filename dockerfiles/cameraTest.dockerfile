FROM debian:bookworm-slim

WORKDIR /test


RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    curl gnupg ca-certificates \
    && \
    curl -fsSL https://archive.raspberrypi.org/debian/raspberrypi.gpg.key | gpg --dearmor -o /usr/share/keyrings/raspberrypi-archive-keyring.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/raspberrypi-archive-keyring.gpg] http://archive.raspberrypi.org/debian/ bookworm main" > /etc/apt/sources.list.d/raspi.list && \
    apt-get update


COPY . /test/

RUN ./bash/req.sh && \
    rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/bin/bash"]
