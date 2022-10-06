FROM gcr.io/oss-fuzz-base/base-builder:v1
RUN apt-get update && apt-get install -y cmake make libpcre2-dev libgtest-dev
COPY . $SRC/pcre2_ast
WORKDIR pcre2_ast
COPY .clusterfuzzlite/build.sh $SRC/
COPY fuzzer.cc $SRC/
