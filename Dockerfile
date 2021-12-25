#
# Build Image
#
ARG ALPINE_VERSION=3.15.0

FROM alpine:${ALPINE_VERSION} AS alpine

#
# Build environment
#

FROM alpine as buildenv

# Dependencies needed for building Mapcrafter
# (not sure how many of these are actually needed)
RUN apk add \
        cmake \
        gcc \
        make \
        g++ \
        zlib-dev \
        libpng-dev \
        libjpeg-turbo-dev \
        boost-dev

#
# Build Mapcrafter from source
#

FROM buildenv as builder

# Add the git repo
ADD . /git/mapcrafter

# Build mapcrafter from source
RUN cd /git/mapcrafter && \
    mkdir build && cd build && \
    cmake .. && \
    make && \
    mkdir /tmp/mapcrafter && \
    make DESTDIR=/tmp/mapcrafter install

#
# Final Image
#

FROM alpine

# Mapcrafter, built in previous stage
COPY --from=builder /tmp/mapcrafter/ /

# Depedencies needed for running Mapcrafter
RUN apk add \
        libpng \
        libjpeg-turbo \
        boost \
        boost-iostreams \
        boost-system \
        boost-filesystem \
        boost-program_options \
        shadow

# Entrypoint
ADD entrypoint.sh /
ADD marker_entrypoint.sh /
ENTRYPOINT ["/entrypoint.sh"]
