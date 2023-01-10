FROM ubuntu:20.04

# Prepare Timezone setting
ENV TZ=Asia/Seoul
ENV TERM=linux
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends cmake bc ninja-build git openssh-client g++10 curl wget unzip software-properties-common diffutils patch make file elfutils vim
RUN apt-get install -y --no-install-recommends libwayland-dev libegl1-mesa-dev build-essential libinput-dev libdrm-dev libgbm-dev
RUN mkdir -m 700 /root/.ssh; \
    touch -m 600 /root/.ssh/known_hosts; \
    ssh-keyscan github.com > /root/.ssh/known_hosts
ENV HOME=/root
COPY vimrc /root/.vimrc
WORKDIR /root

ENV XDG_RUNTIME_DIR=/var/run
CMD ["/bin/bash"]
