FROM --platform=linux/arm64 debian:bookworm AS builder

RUN apt-get update && apt-get install -y build-essential cmake libgps-dev && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build \
 && cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Release} .. \
 && make -j$(nproc)

FROM --platform=linux/arm64 debian:bookworm-slim

RUN apt-get update && apt-get install -y libgps23 && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/awns-rpi5 /usr/local/bin/awns-rpi5

ENTRYPOINT ["awns-rpi5"]
