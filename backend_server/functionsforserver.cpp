/**
 * @file functionsforserver.cpp
 * @brief Реализация всех алгоритмов сервера.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 */

#include "functionsforserver.h"

#include <QString>
#include <QStringList>
#include "sha384.h"
#include <QFile>
#include <QByteArray>
#include <cmath>
#include <limits>

// ─────────────────────────────────────────────
//  ПАРСИНГ КОМАНД
// ─────────────────────────────────────────────

QString parsing(QString request)
{
    request = request.trimmed(); // убираем пробелы и переносы строк по краям

    QStringList parts = request.split(":"); // разбиваем по символу ':'

    if (parts.isEmpty()) // пустой запрос — возвращаем ошибку
        return "error";

    QString cmd = parts[0].toLower(); // первый элемент — команда (в нижнем регистре)

    // --- Шифр Виженера ---
    if (cmd == "vigenere_encrypt") {
        if (parts.size() < 3) return "error"; // нужно минимум 3 части: команда, текст, ключ
        return vigenereEncrypt(parts[1], parts[2]); // parts[1] = текст, parts[2] = ключ
    }
    if (cmd == "vigenere_decrypt") {
        if (parts.size() < 3) return "error";
        return vigenereDecrypt(parts[1], parts[2]);
    }

    // --- SHA-384 ---
    if (cmd == "sha384") {
        if (parts.size() < 2) return "error"; // нужно: команда + текст
        return sha384(parts[1]);
    }

    // --- Метод хорд ---
    if (cmd == "chord") {
        if (parts.size() < 4) return "error"; // нужно: команда + a + b + eps
        return chordMethodStr(parts[1], parts[2], parts[3]);
    }

    // --- Стеганография ---
    if (cmd == "stego_encode") {
        if (parts.size() < 3) return "error"; // нужно: команда + путь + сообщение
        return stegoEncode(parts[1], parts[2]);
    }
    if (cmd == "stego_decode") {
        if (parts.size() < 2) return "error"; // нужно: команда + путь
        return stegoDecode(parts[1]);
    }

    return "error"; // команда не распознана
}

// ─────────────────────────────────────────────
//  ШИФР ВИЖЕНЕРА
// ─────────────────────────────────────────────

QString vigenereEncrypt(const QString &text, const QString &key)
{
    if (key.isEmpty()) return "error"; // пустой ключ — ошибка

    QString result;                          // строка для результата
    QString upperKey = key.toUpper();        // ключ переводим в верхний регистр
    int keyLen = upperKey.length();          // длина ключа
    int keyIdx = 0;                          // текущая позиция в ключе

    for (int i = 0; i < text.length(); i++) // проходим по каждому символу текста
    {
        QChar c = text[i]; // текущий символ

        if (c.isLetter()) // обрабатываем только буквы
        {
            bool isUpper = c.isUpper(); // запоминаем регистр символа
            int base = isUpper ? 'A' : 'a'; // база: 'A'=65 для верхнего, 'a'=97 для нижнего

            // получаем сдвиг из ключа: буква ключа минус 'A' = число от 0 до 25
            int shift = upperKey[keyIdx % keyLen].toLatin1() - 'A';

            // применяем формулу Виженера: сдвигаем букву на shift позиций
            // % 26 — чтобы не выйти за пределы алфавита
            int encrypted = (c.toLatin1() - base + shift) % 26 + base;

            result += QChar(encrypted); // добавляем зашифрованный символ
            keyIdx++;                   // переходим к следующей букве ключа
        }
        else
        {
            result += c; // нелатинские символы добавляем без изменений
        }
    }
    return result;
}

QString vigenereDecrypt(const QString &text, const QString &key)
{
    if (key.isEmpty()) return "error";

    QString result;
    QString upperKey = key.toUpper();
    int keyLen = upperKey.length();
    int keyIdx = 0;

    for (int i = 0; i < text.length(); i++)
    {
        QChar c = text[i];

        if (c.isLetter())
        {
            bool isUpper = c.isUpper();
            int base  = isUpper ? 'A' : 'a';

            int shift = upperKey[keyIdx % keyLen].toLatin1() - 'A';

            // дешифрование: вычитаем сдвиг
            // +26 чтобы избежать отрицательных значений при взятии остатка
            int decrypted = (c.toLatin1() - base - shift + 26) % 26 + base;

            result += QChar(decrypted);
            keyIdx++;
        }
        else
        {
            result += c;
        }
    }
    return result;
}

// ─────────────────────────────────────────────
//  SHA-384
// ─────────────────────────────────────────────

QString sha384(const QString &text)
{
    // используем нашу собственную реализацию SHA-384
    // без сторонних библиотек — полностью самостоятельный алгоритм
    return SHA384::hash(text);
}

// ─────────────────────────────────────────────
//  МЕТОД ХОРД
// ─────────────────────────────────────────────

/**
 * @brief Уравнение для нахождения корня.
 * @param x Значение аргумента
 * @return Значение f(x) = x^3 - x - 2
 */
static double f(double x)
{
    return x * x * x - x - 2.0; // f(x) = x³ - x - 2
}

double chordMethod(double a, double b, double eps)
{
    // проверяем что на концах отрезка функция имеет разные знаки
    // если f(a) и f(b) одного знака — корня на отрезке нет
    if (f(a) * f(b) > 0)
        return std::numeric_limits<double>::quiet_NaN(); // возвращаем NaN

    double x = a;        // начальное приближение
    int maxIter = 10000; // максимальное число итераций (защита от бесконечного цикла)

    for (int i = 0; i < maxIter; i++)
    {
        double fa = f(a); // значение функции в точке a
        double fb = f(b); // значение функции в точке b

        // формула метода хорд — находим точку пересечения хорды с осью X
        x = a - fa * (b - a) / (fb - fa);

        double fx = f(x); // значение функции в новой точке

        if (std::fabs(fx) < eps) // если достигли нужной точности — выходим
            break;

        // сужаем отрезок: выбираем ту половину где функция меняет знак
        if (fa * fx < 0)
            b = x; // корень в левой половине [a, x]
        else
            a = x; // корень в правой половине [x, b]
    }

    return x; // возвращаем найденный корень
}

QString chordMethodStr(const QString &a, const QString &b, const QString &eps)
{
    bool okA, okB, okE;

    // конвертируем строки в числа с проверкой успеха
    double da = a.toDouble(&okA);
    double db = b.toDouble(&okB);
    double de = eps.toDouble(&okE);

    if (!okA || !okB || !okE) return "error"; // конвертация не удалась
    if (de <= 0) return "error";              // точность должна быть положительной

    double result = chordMethod(da, db, de);

    if (std::isnan(result))
        return "error: no root on interval"; // корня нет на данном отрезке

    // возвращаем результат с точностью 10 знаков после запятой
    return QString::number(result, 'f', 10);
}

// ─────────────────────────────────────────────
//  СТЕГАНОГРАФИЯ (LSB в BMP)
// ─────────────────────────────────────────────

QString stegoEncode(const QString &imagePath, const QString &message)
{
    QFile file(imagePath);

    // открываем файл для чтения и записи
    if (!file.open(QIODevice::ReadWrite))
        return "error: cannot open file";

    QByteArray data = file.readAll(); // читаем все байты файла

    const int BMP_HEADER_SIZE = 54; // размер заголовка BMP всегда 54 байта

    if (data.size() < BMP_HEADER_SIZE)
        return "error: file too small"; // файл слишком мал — не BMP

    QByteArray msgBytes = message.toUtf8(); // сообщение в байтах
    int msgLen = msgBytes.size();           // длина сообщения в байтах

    // проверяем что изображение достаточно большое для сообщения
    // нужно: (4 байта длины + байты сообщения) * 8 бит = количество пикселей
    int bitsNeeded = (4 + msgLen) * 8;
    int availableBytes = data.size() - BMP_HEADER_SIZE; // доступные байты пикселей

    if (availableBytes < bitsNeeded)
        return "error: image too small for this message";

    int byteIdx = BMP_HEADER_SIZE; // начинаем с первого байта пикселей (пропускаем заголовок)

    // записываем длину сообщения в первые 32 бита (4 байта по 1 биту в каждом пикселе)
    for (int i = 31; i >= 0; i--)
    {
        int bit = (msgLen >> i) & 1; // извлекаем i-й бит длины сообщения

        // 0xFE = 11111110 — обнуляем младший бит
        // | bit — устанавливаем нужное значение
        data[byteIdx] = (data[byteIdx] & 0xFE) | bit;
        byteIdx++; // переходим к следующему байту пикселя
    }

    // записываем каждый байт сообщения побитово
    for (int i = 0; i < msgLen; i++)
    {
        unsigned char ch = static_cast<unsigned char>(msgBytes[i]); // текущий байт сообщения

        for (int bit = 7; bit >= 0; bit--) // обрабатываем каждый из 8 бит байта
        {
            int b = (ch >> bit) & 1; // извлекаем текущий бит

            data[byteIdx] = (data[byteIdx] & 0xFE) | b; // записываем бит в LSB пикселя
            byteIdx++;
        }
    }

    file.seek(0);       // перемещаемся в начало файла
    file.write(data);   // записываем изменённые данные
    file.close();

    return "ok: message encoded";
}

QString stegoDecode(const QString &imagePath)
{
    QFile file(imagePath);

    if (!file.open(QIODevice::ReadOnly))
        return "error: cannot open file";

    QByteArray data = file.readAll();
    file.close();

    const int BMP_HEADER_SIZE = 54;

    if (data.size() < BMP_HEADER_SIZE)
        return "error: file too small";

    int byteIdx = BMP_HEADER_SIZE; // пропускаем заголовок BMP

    // читаем длину сообщения из первых 32 бит
    int msgLen = 0;
    for (int i = 0; i < 32; i++)
    {
        // сдвигаем накопленное значение влево и добавляем LSB текущего байта
        msgLen = (msgLen << 1) | (data[byteIdx] & 1);
        byteIdx++;
    }

    // проверяем что длина разумная
    if (msgLen <= 0 || msgLen > 100000)
        return "error: invalid message length";

    if (byteIdx + msgLen * 8 > data.size())
        return "error: data corrupted";

    // восстанавливаем байты сообщения из LSB пикселей
    QByteArray msgBytes;
    for (int i = 0; i < msgLen; i++)
    {
        unsigned char ch = 0; // текущий байт сообщения

        for (int bit = 0; bit < 8; bit++) // собираем 8 бит в один байт
        {
            ch = (ch << 1) | (data[byteIdx] & 1); // сдвиг и добавление LSB
            byteIdx++;
        }

        msgBytes.append(static_cast<char>(ch)); // добавляем восстановленный байт
    }

    return QString::fromUtf8(msgBytes); // переводим байты обратно в строку
}
