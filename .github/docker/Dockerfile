FROM ubuntu:18.04

# Install needed packages
RUN apt update && apt install -y \
    apt-transport-https \
    build-essential \
    ca-certificates \
    clang-10 \
    curl \
    default-jdk \
    freeglut3 \
    freeglut3-dev \
    g++-8 \
    gcc-8 \
    gnupg \
    gzip \
    help2man \
    lcov \
    libgl1-mesa-dev \
    libglfw3-dev \
    libglu1-mesa \
    libjson-perl \
    libperlio-gzip-perl \
    libqt5opengl5-dev \
    libx11-dev \
    libxcursor1 \
    libxt-dev \
    openssh-client \
    python3-dev \
    qtbase5-dev \
    software-properties-common \
    wget \
    xvfb

# Install uptodate needed packages
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
RUN add-apt-repository ppa:git-core/ppa
RUN apt update && apt install -y cmake git git-lfs

# Install a more recent lcov
RUN wget http://archive.ubuntu.com/ubuntu/pool/universe/l/lcov/lcov_1.14-2_all.deb
RUN dpkg -i lcov_1.14-2_all.deb

# Use gcc-8 by default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8
