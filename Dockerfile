# =============================================================================
# IrineCore — OpenSource Dragon Nest Server Emulator
# Multi-stage Dockerfile (Ubuntu 24.04 LTS Builder -> Runtime Dist in /irinecore/env/dist)
# =============================================================================
FROM ubuntu:26.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    ca-certificates \
    unixodbc-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -S /src -B /src/build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/src/env/dist \
    && cmake --build /src/build --parallel \
    && cmake --install /src/build

# =============================================================================
# Runtime Stage
# =============================================================================
FROM ubuntu:26.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
    unixodbc \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /irinecore/env/dist
COPY --from=builder /src/env/dist /irinecore/env/dist
COPY apps/docker/entrypoint.sh /usr/local/bin/irine-entrypoint.sh
RUN chmod +x /usr/local/bin/irine-entrypoint.sh

EXPOSE 14300 14400 15100/udp

ENTRYPOINT ["/usr/local/bin/irine-entrypoint.sh"]
CMD ["DNLoginServer64"]
