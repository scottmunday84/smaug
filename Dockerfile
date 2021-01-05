FROM ubuntu AS builder
RUN apt-get update && apt-get install -y build-essential
RUN rm -rf /var/lib/apt/lists/*
COPY ./paradox /paradox
WORKDIR /paradox/src
RUN make

FROM ubuntu
COPY --from=builder /paradox /paradox
WORKDIR /paradox/area
CMD ln -s /paradox/src/rot /paradox/area/rot && ./rot
EXPOSE 8080
EXPOSE 8081
