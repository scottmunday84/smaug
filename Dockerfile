FROM ubuntu AS builder
RUN apt-get update && apt-get install -y build-essential
RUN rm -rf /var/lib/apt/lists/*
COPY ./paradox /paradox
WORKDIR /paradox/src
RUN make

FROM ubuntu
COPY --from=builder /paradox /paradox
WORKDIR /paradox/area
RUN mv /paradox/src/rot /paradox/area/startup
CMD tail -f /dev/null
EXPOSE 4000

