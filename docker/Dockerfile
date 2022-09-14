# -----------------------------------------------------------------------------
# Buildtime
# -----------------------------------------------------------------------------
FROM ubuntu:latest as build

LABEL maintainer="Rainer Schoenberger <rschoe@de.ibm.com>"

ENV DEBIAN_FRONTEND noninteractive
RUN apt update
RUN apt install -y cmake \
                    make \
                    g++ \
                    git \
                    zlib1g-dev   `# gRPC needs this to build and dynamically link` \
                    bash         `# for generateVersionDefine script` \
                    openssl      `# for generating example server keys` \
                    libssl-dev   `# for gRPC to link against on some target platforms which cannot use boringssl (e.g. s390)`

ADD . /gwhisper
RUN  rm -rf /gwhisper/build  && mkdir  /gwhisper/build
WORKDIR /gwhisper/build
RUN cmake .. -DGWHISPER_FORCE_BUILDING_GRPC=ON -DGWHISPER_BUILD_TESTS=ON
RUN cmake --build . --parallel 8

# FIXME: ssl related functional-tests fail for s390x platform, when run in QEMU
#        (via docker buildx)
#        This is caused by QEMU being quite slow, causing connect timeout to hit
#RUN ctest --verbose

# -----------------------------------------------------------------------------
# Runtime
# -----------------------------------------------------------------------------
FROM ubuntu:latest

RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt install -y zlib1g-dev \
                                                  bash \
                                                  fish \
                                                  libssl-dev

COPY --from=build /gwhisper/build/bin/gwhisper /bin/
COPY --from=build /gwhisper/build/bin/testServer /bin/gwhisper_testServer
COPY --from=build /gwhisper/complete.bash /usr/share/bash-completion/completions/gwhisper
COPY --from=build /gwhisper/complete.fish /usr/share/fish/vendor_completions.d/gwhisper.fish

CMD /usr/bin/fish

# uncomment below if you want to have basic development support in the container:
# # install development tools/dependencies:
# RUN DEBIAN_FRONTEND=noninteractive apt install -y cmake \
#                                                   make \
#                                                   gdb \
#                                                   g++ \
#                                                   openssl \
#                                                   vim \
#                                                   netcat
# # copy sourcecode and build into the container:
# COPY --from=build /gwhisper /gwhisper
