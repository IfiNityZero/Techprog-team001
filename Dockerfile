FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qttools5-dev \
    build-essential \
    qt5-qmake \
    libqt5network5 \
    libqt5sql5-sqlite \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cd backend_server && \
    qmake echoServer.pro && \
    make && \
    echo "Build successful!" && \
    ls -la

EXPOSE 33333

CMD ["./backend_server/echoServer"]
