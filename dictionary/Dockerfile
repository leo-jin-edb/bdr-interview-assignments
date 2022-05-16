FROM gcc:12 as builder
WORKDIR /app
COPY *.c /app/
COPY Makefile /app/
RUN mkdir -p obj && make dict

FROM debian:bullseye-slim
WORKDIR /app
COPY --from=builder /app/dict /app/
CMD ["/app/dict"]