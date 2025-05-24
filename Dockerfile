FROM --platform=linux/arm64 debian:bookworm AS builder

RUN apt-get update && apt-get install -y build-essential cmake libgps-dev && rm -rf /var/lib/apt/lists/* && apt-cache policy libgps-dev

WORKDIR /app

COPY . .

RUN cd build \
 && cmake -DCMAKE_BUILD_TYPE=release .. \
 && cmake --build .

FROM --platform=linux/arm64 debian:bookworm-slim

RUN apt-get update && apt-get install -y libgps28 && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/awns-rpi5 /usr/local/bin/awns-rpi5

ENTRYPOINT ["awns-rpi5"]
