FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем Qt5 (более стабильный в Ubuntu 22.04)
RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qttools5-dev \
    build-essential \
    qt5-qmake \
    libqt5network5 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем весь проект
COPY . .

# Добавляем functionsforserver в .pro файл и собираем
RUN cd backend_server && \
    echo "SOURCES += functionsforserver.cpp" >> echoServer.pro && \
    echo "HEADERS += functionsforserver.h" >> echoServer.pro && \
    qmake echoServer.pro && \
    make && \
    echo "Build successful!" && \
    ls -la

EXPOSE 33333

CMD ["./backend_server/echoServer"]
