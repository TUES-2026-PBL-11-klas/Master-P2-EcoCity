FROM ubuntu:24.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        pkg-config \
        protobuf-compiler \
        libprotobuf-dev \
        libmongocxx-dev \
        libbsoncxx-dev \
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

COPY game_api/ game_api/

COPY game-engine/ game-engine/

WORKDIR /workspace/game-engine
RUN make -j"$(nproc)"

RUN make test  -j"$(nproc)"
RUN make run-tests

FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        libprotobuf32t64 \
        libmongoc-1.0-0 \
        libbson-1.0-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /workspace/game-engine/eco_city_engine .

COPY --from=builder /usr/lib/x86_64-linux-gnu/libmongocxx*.so* /usr/lib/x86_64-linux-gnu/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libbsoncxx*.so* /usr/lib/x86_64-linux-gnu/
RUN ldconfig

EXPOSE 54321

ENTRYPOINT ["./eco_city_engine"]
