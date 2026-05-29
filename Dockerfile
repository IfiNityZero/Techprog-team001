# Используем Ubuntu 22.04 как базовый образ
FROM ubuntu:22.04

# Отключаем интерактивные вопросы при установке пакетов
ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем Qt и необходимые зависимости
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-tools-dev \
    cmake \
    build-essential \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# Создаём рабочую директорию внутри контейнера
WORKDIR /app

# Копируем весь проект в контейнер
COPY . .

# Собираем сервер
RUN cd backend_server && \
    qmake6 echoServer.pro && \
    make

# Порт на котором слушает сервер
EXPOSE 33333

# Запускаем сервер
CMD ["./backend_server/EchoServer"]
