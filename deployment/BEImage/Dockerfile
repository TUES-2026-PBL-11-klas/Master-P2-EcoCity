FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY /game-engine ./

WORKDIR /app/game-engine

RUN make

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder eco_city_engine.out .

RUN chmod +x ./eco_city_engine.out

CMD ["./eco_city_engine.out"]
