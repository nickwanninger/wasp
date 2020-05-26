FROM clux/muslrust

# install cargo build deps
WORKDIR /app
RUN cargo install cargo-build-dependencies
COPY ./Cargo.* ./
RUN mkdir -p ./src && echo 'fn main(){}' > ./src/main.rs
RUN cargo build-dependencies --release

# copy the rest of the source files
COPY ./src ./src
RUN cargo build --release --color always  # build app
