# =============================================================================
# IrineCore — OpenSource Dragon Nest Server Emulator
# Multi-stage Dockerfile (Ubuntu 24.04 LTS Builder -> Runtime Dist in /irinecore/env/dist)
# =============================================================================
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ccache \
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
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_INSTALL_PREFIX=/src/env/dist \
    && cmake --build /src/build --parallel \
    && cmake --install /src/build

# =============================================================================
# Runtime Stage (with Microsoft ODBC Driver 18 for SQL Server & healthcheck tools)
# =============================================================================
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
    netcat-openbsd \
    procps \
    unixodbc \
    zlib1g \
    && mkdir -p /etc/apt/keyrings \
    && curl -fsSL https://packages.microsoft.com/keys/microsoft.asc \
        | gpg --dearmor -o /etc/apt/keyrings/microsoft.gpg \
    && echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/microsoft.gpg] https://packages.microsoft.com/ubuntu/24.04/prod noble main" \
        > /etc/apt/sources.list.d/mssql-release.list \
    && apt-get update \
    && ACCEPT_EULA=Y apt-get install -y --no-install-recommends msodbcsql18 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /irinecore/env/dist
COPY --from=builder /src/env/dist /irinecore/env/dist
COPY apps/docker/entrypoint.sh /usr/local/bin/irine-entrypoint.sh
RUN sed -i 's/\r$//' /usr/local/bin/irine-entrypoint.sh \
    && chmod +x /usr/local/bin/irine-entrypoint.sh

EXPOSE 3000 2600 12600 7000 8000 9000 14300 14400 15100-15110/udp

ENTRYPOINT ["/usr/local/bin/irine-entrypoint.sh"]
CMD ["DNLoginServer64"]
