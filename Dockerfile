FROM ubuntu:24.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        pkg-config \
        protobuf-compiler \
        libprotobuf-dev \
        libgtest-dev \
        cmake \
        curl \
        gnupg \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://www.mongodb.org/static/pgp/server-7.0.asc \
        | gpg --dearmor -o /usr/share/keyrings/mongodb-server-7.0.gpg \
    && echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-7.0.gpg ] \
        https://repo.mongodb.org/apt/ubuntu noble/mongodb-org/7.0 multiverse" \
        > /etc/apt/sources.list.d/mongodb-org-7.0.list \
    && apt-get update && apt-get install -y --no-install-recommends \
        libmongocxx-dev \
        libbsoncxx-dev \
    && rm -rf /var/lib/apt/lists/*

RUN cmake -S /usr/src/googletest \
          -B /tmp/gtest-build \
          -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=ON \
    && cmake --build /tmp/gtest-build --parallel "$(nproc)" \
    && cmake --install /tmp/gtest-build \
    && ldconfig \
    && rm -rf /tmp/gtest-build

WORKDIR /workspace
COPY game_api/   game_api/
COPY game-engine/ game-engine/

WORKDIR /workspace/game-engine
RUN make -j"$(nproc)"

RUN make test -j"$(nproc)"
RUN make run-tests

FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        libprotobuf32t64 \
        libmongoc-1.0-0 \
        libbson-1.0-0 \
        curl \
        gnupg \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://www.mongodb.org/static/pgp/server-7.0.asc \
        | gpg --dearmor -o /usr/share/keyrings/mongodb-server-7.0.gpg \
    && echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-7.0.gpg ] \
        https://repo.mongodb.org/apt/ubuntu noble/mongodb-org/7.0 multiverse" \
        > /etc/apt/sources.list.d/mongodb-org-7.0.list \
    && apt-get update && apt-get install -y --no-install-recommends \
        libmongocxx \
        libbsoncxx \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get purge -y --auto-remove curl gnupg

WORKDIR /app
COPY --from=builder /workspace/game-engine/eco_city_engine .

EXPOSE 54321
ENTRYPOINT ["./eco_city_engine"]
