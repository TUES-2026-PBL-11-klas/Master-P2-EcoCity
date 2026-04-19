FROM rust:1.94.1-slim-trixie
WORKDIR /usr/src/app/UI

COPY UI/Cargo.toml UI/Cargo.lock ./

RUN mkdir src && echo "fn main() {}" > src/main.rs

RUN cargo build --release

RUN rm -rf src
COPY UI/src ./src
COPY UI/assets ./assets

RUN cargo build --release

CMD ["./target/release/UI"]
