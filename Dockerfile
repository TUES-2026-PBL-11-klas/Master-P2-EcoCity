FROM ubuntu:24.04 AS deps

ARG DEBIAN_FRONTEND=noninteractive
ARG MONGOC_VERSION=1.26.0
ARG MONGOCXX_VERSION=3.10.1

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        pkg-config \
        libssl-dev \
        libsasl2-dev \
        curl \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://github.com/mongodb/mongo-c-driver/releases/download/${MONGOC_VERSION}/mongo-c-driver-${MONGOC_VERSION}.tar.gz \
        | tar -xz -C /tmp \
    && cmake -S /tmp/mongo-c-driver-${MONGOC_VERSION} \
             -B /tmp/mongoc-build \
             -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_INSTALL_PREFIX=/usr/local \
             -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF \
             -DENABLE_TESTS=OFF \
             -DENABLE_EXAMPLES=OFF \
    && cmake --build /tmp/mongoc-build --parallel "$(nproc)" \
    && cmake --install /tmp/mongoc-build \
    && rm -rf /tmp/mongoc-build /tmp/mongo-c-driver-${MONGOC_VERSION}

RUN curl -fsSL https://github.com/mongodb/mongo-cxx-driver/releases/download/r${MONGOCXX_VERSION}/mongo-cxx-driver-r${MONGOCXX_VERSION}.tar.gz \
        | tar -xz -C /tmp \
    && cmake -S /tmp/mongo-cxx-driver-r${MONGOCXX_VERSION} \
             -B /tmp/mongocxx-build \
             -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_INSTALL_PREFIX=/usr/local \
             -DCMAKE_PREFIX_PATH=/usr/local \
             -DENABLE_TESTS=OFF \
    && cmake --build /tmp/mongocxx-build --parallel "$(nproc)" \
    && cmake --install /tmp/mongocxx-build \
    && rm -rf /tmp/mongocxx-build /tmp/mongo-cxx-driver-r${MONGOCXX_VERSION}


FROM deps AS builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        pkg-config \
        protobuf-compiler \
        libprotobuf-dev \
        libgtest-dev \
        cmake \
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
COPY game_api/    game_api/
COPY game-engine/ game-engine/

WORKDIR /workspace/game-engine
RUN make -j"$(nproc)"
RUN make test -j"$(nproc)"
RUN make run-tests


FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        libprotobuf32t64 \
        libssl3t64 \
        libsasl2-2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /workspace/game-engine/eco_city_engine .
COPY --from=deps /usr/local/lib/libmongoc-1.0.so*  /usr/local/lib/
COPY --from=deps /usr/local/lib/libbson-1.0.so*    /usr/local/lib/
COPY --from=deps /usr/local/lib/libmongocxx.so*    /usr/local/lib/
COPY --from=deps /usr/local/lib/libbsoncxx.so*     /usr/local/lib/
RUN ldconfig

EXPOSE 54321
ENTRYPOINT ["./eco_city_engine"]
