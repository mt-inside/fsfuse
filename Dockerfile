FROM ubuntu:16.10

RUN apt update
# Build
RUN apt install -y build-essential xsltproc libcurl4-openssl-dev libfuse-dev libxml2-dev check
# Dev / Debug
RUN apt install -y vim git man gdb valgrind
