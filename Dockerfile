FROM ubuntu AS builder
RUN apt-get update && apt-get install -y build-essential
COPY ./paradox /paradox
WORKDIR /paradox/src
RUN make

FROM ubuntu
COPY --from=builder /paradox /
CMD ['startup']
EXPOSE 4000

