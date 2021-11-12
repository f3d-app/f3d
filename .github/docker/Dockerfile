# Latest Ubuntu based docker image from nvidia
FROM nvidia/opengl:1.2-glvnd-devel-ubuntu18.04 AS Builder

# Install needed packages
RUN apt update && apt install -y \
    apt-transport-https \
    build-essential \
    ca-certificates \
    curl \
    gnupg \
    gzip \
    help2man \
    lcov \
    libx11-dev \
    openssh-client \
    software-properties-common \
    wget \
    xvfb 

# Install uptodate needed packages
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
RUN add-apt-repository ppa:git-core/ppa
RUN apt update && apt install -y cmake git git-lfs
