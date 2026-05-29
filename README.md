# Techprog-team001

## Проект по дисциплине "Технологии и методы программирования"

### Участники (учебная группа: 251-254)

1. Усачев Тимофей Владимирович
2. Корвяков Святогор
3. Гусейнов Артем

---

### Вариант команды

| Компонент | Реализация |
|-----------|-----------|
| Шифрование | Шифр Виженера |
| Хеш | SHA-384 |
| Численный метод | Метод хорд (уравнение: x³ − x − 2 = 0) |
| Задача | Внедрение сообщения в картинку (стеганография, LSB в BMP) |

---

### Структура проекта

```
TaMP_proj/
├── backend_server/       # TCP сервер на Qt
│   ├── echoServer.pro
│   ├── main.cpp
│   ├── mytcpserver.h/.cpp
│   ├── functionsforserver.h/.cpp   # реализация алгоритмов
├── frontend_client/      # Qt GUI клиент
├── test_functionality/   # Модульные тесты
│   ├── UnitTests.pro
│   └── tst_funcforserver_test.cpp
├── DataBase/             # Работа с БД (SQLite)
│   ├── DataBase.pro
│   └── main.cpp
├── Dockerfile
└── README.md
```

---

### Формат команд сервера

Клиент отправляет текстовые команды, сервер отвечает результатом:

| Команда | Пример | Ответ |
|---------|--------|-------|
| `vigenere_encrypt:ТЕКСТ:КЛЮЧ` | `vigenere_encrypt:HELLO:KEY` | `RIJVS` |
| `vigenere_decrypt:ТЕКСТ:КЛЮЧ` | `vigenere_decrypt:RIJVS:KEY` | `HELLO` |
| `sha384:ТЕКСТ` | `sha384:abc` | `cb00753f...` |
| `chord:A:B:EPS` | `chord:1:2:0.000001` | `1.5213797068` |
| `stego_encode:ПУТЬ:СООБЩЕНИЕ` | `stego_encode:/img/a.bmp:hello` | `ok: message encoded` |
| `stego_decode:ПУТЬ` | `stego_decode:/img/a.bmp` | `hello` |

---

### Сборка и запуск через Docker

```bash
# Сборка образа
docker build -t tamp_proj .

# Запуск сервера
docker run -p 33333:33333 tamp_proj
```

### Сборка без Docker (Qt установлен локально)

```bash
cd backend_server
qmake echoServer.pro
make
./EchoServer
```
