#include "functionsforserver.h"

#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QFile>
#include <QByteArray>
#include <cmath>

// ─────────────────────────────────────────────
//  ПАРСИНГ КОМАНД
//  Формат запросов от клиента:
//    vigenere_encrypt:ТЕКСТ:КЛЮЧ
//    vigenere_decrypt:ТЕКСТ:КЛЮЧ
//    sha384:ТЕКСТ
//    chord:A:B:EPS
//    stego_encode:ПУТЬ_К_ФАЙЛУ:СООБЩЕНИЕ
//    stego_decode:ПУТЬ_К_ФАЙЛУ
// ─────────────────────────────────────────────
QString parsing(QString request)
{
    request = request.trimmed();
    QStringList parts = request.split(":");

    if (parts.isEmpty())
        return "error";

    QString cmd = parts[0].toLower();

    // --- Шифр Виженера ---
    if (cmd == "vigenere_encrypt") {
        if (parts.size() < 3) return "error";
        return vigenereEncrypt(parts[1], parts[2]);
    }
    if (cmd == "vigenere_decrypt") {
        if (parts.size() < 3) return "error";
        return vigenereDecrypt(parts[1], parts[2]);
    }

    // --- SHA-384 ---
    if (cmd == "sha384") {
        if (parts.size() < 2) return "error";
        return sha384(parts[1]);
    }

    // --- Метод хорд ---
    if (cmd == "chord") {
        if (parts.size() < 4) return "error";
        return chordMethodStr(parts[1], parts[2], parts[3]);
    }

    // --- Стеганография ---
    if (cmd == "stego_encode") {
        if (parts.size() < 3) return "error";
        return stegoEncode(parts[1], parts[2]);
    }
    if (cmd == "stego_decode") {
        if (parts.size() < 2) return "error";
        return stegoDecode(parts[1]);
    }

    return "error";
}

// ─────────────────────────────────────────────
//  ШИФР ВИЖЕНЕРА
//  Работает только с латинскими буквами (A-Z, a-z).
//  Всё остальное (цифры, пробелы) остаётся без изменений.
// ─────────────────────────────────────────────
QString vigenereEncrypt(const QString &text, const QString &key)
{
    if (key.isEmpty()) return "error";

    QString result;
    QString upperKey = key.toUpper();
    int keyLen = upperKey.length();
    int keyIdx = 0;

    for (int i = 0; i < text.length(); i++) {
        QChar c = text[i];
        if (c.isLetter()) {
            bool isUpper = c.isUpper();
            int base = isUpper ? 'A' : 'a';
            int shift = upperKey[keyIdx % keyLen].toLatin1() - 'A';
            int encrypted = (c.toLatin1() - base + shift) % 26 + base;
            result += QChar(encrypted);
            keyIdx++;
        } else {
            result += c;
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

    for (int i = 0; i < text.length(); i++) {
        QChar c = text[i];
        if (c.isLetter()) {
            bool isUpper = c.isUpper();
            int base = isUpper ? 'A' : 'a';
            int shift = upperKey[keyIdx % keyLen].toLatin1() - 'A';
            int decrypted = (c.toLatin1() - base - shift + 26) % 26 + base;
            result += QChar(decrypted);
            keyIdx++;
        } else {
            result += c;
        }
    }
    return result;
}

// ─────────────────────────────────────────────
//  SHA-384
//  Использует встроенный Qt модуль QCryptographicHash
// ─────────────────────────────────────────────
QString sha384(const QString &text)
{
    QByteArray inputBytes = text.toUtf8();
    QByteArray hashBytes = QCryptographicHash::hash(inputBytes, QCryptographicHash::Sha384);
    return QString(hashBytes.toHex());
}

// ─────────────────────────────────────────────
//  МЕТОД ХОРД
//  Уравнение: f(x) = x^3 - x - 2 = 0
//  Корень находится на отрезке [a, b] с точностью eps
// ─────────────────────────────────────────────
static double f(double x)
{
    return x * x * x - x - 2.0;
}

double chordMethod(double a, double b, double eps)
{
    // Проверяем что на концах отрезка функция имеет разные знаки
    if (f(a) * f(b) > 0)
        return std::numeric_limits<double>::quiet_NaN();

    double x = a;
    int maxIter = 10000;

    for (int i = 0; i < maxIter; i++) {
        double fa = f(a);
        double fb = f(b);

        // Формула метода хорд
        x = a - fa * (b - a) / (fb - fa);

        double fx = f(x);

        if (std::fabs(fx) < eps)
            break;

        // Сужаем отрезок
        if (fa * fx < 0)
            b = x;
        else
            a = x;
    }

    return x;
}

QString chordMethodStr(const QString &a, const QString &b, const QString &eps)
{
    bool okA, okB, okE;
    double da = a.toDouble(&okA);
    double db = b.toDouble(&okB);
    double de = eps.toDouble(&okE);

    if (!okA || !okB || !okE) return "error";
    if (de <= 0) return "error";

    double result = chordMethod(da, db, de);

    if (std::isnan(result))
        return "error: no root on interval";

    return QString::number(result, 'f', 10);
}

// ─────────────────────────────────────────────
//  СТЕГАНОГРАФИЯ (LSB в BMP)
//  stegoEncode — внедряет сообщение в BMP файл
//  stegoDecode — извлекает сообщение из BMP файла
//
//  Метод: младший бит каждого байта пикселя
//  Формат: первые 32 бита = длина сообщения,
//          затем биты самого сообщения
// ─────────────────────────────────────────────
QString stegoEncode(const QString &imagePath, const QString &message)
{
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadWrite))
        return "error: cannot open file";

    QByteArray data = file.readAll();

    // BMP: первые 54 байта — заголовок, пиксели начинаются с байта 54
    const int BMP_HEADER_SIZE = 54;
    if (data.size() < BMP_HEADER_SIZE)
        return "error: file too small";

    QByteArray msgBytes = message.toUtf8();
    int msgLen = msgBytes.size();

    // Сколько нам нужно байт для записи: 4 байта длины + байты сообщения
    // Каждый бит сообщения записывается в 1 байт пикселя (LSB)
    int bitsNeeded = (4 + msgLen) * 8;
    int availableBytes = data.size() - BMP_HEADER_SIZE;

    if (availableBytes < bitsNeeded)
        return "error: image too small for this message";

    int byteIdx = BMP_HEADER_SIZE;

    // Записываем длину сообщения (4 байта, 32 бита)
    for (int i = 31; i >= 0; i--) {
        int bit = (msgLen >> i) & 1;
        data[byteIdx] = (data[byteIdx] & 0xFE) | bit;
        byteIdx++;
    }

    // Записываем сами байты сообщения побитово
    for (int i = 0; i < msgLen; i++) {
        unsigned char ch = static_cast<unsigned char>(msgBytes[i]);
        for (int bit = 7; bit >= 0; bit--) {
            int b = (ch >> bit) & 1;
            data[byteIdx] = (data[byteIdx] & 0xFE) | b;
            byteIdx++;
        }
    }

    // Записываем обратно в файл
    file.seek(0);
    file.write(data);
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

    int byteIdx = BMP_HEADER_SIZE;

    // Читаем длину сообщения (32 бита)
    int msgLen = 0;
    for (int i = 0; i < 32; i++) {
        msgLen = (msgLen << 1) | (data[byteIdx] & 1);
        byteIdx++;
    }

    if (msgLen <= 0 || msgLen > 100000)
        return "error: invalid message length";

    if (byteIdx + msgLen * 8 > data.size())
        return "error: data corrupted";

    // Читаем байты сообщения
    QByteArray msgBytes;
    for (int i = 0; i < msgLen; i++) {
        unsigned char ch = 0;
        for (int bit = 0; bit < 8; bit++) {
            ch = (ch << 1) | (data[byteIdx] & 1);
            byteIdx++;
        }
        msgBytes.append(static_cast<char>(ch));
    }

    return QString::fromUtf8(msgBytes);
}
