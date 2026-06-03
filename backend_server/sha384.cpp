/**
 * @file sha384.cpp
 * @brief Собственная реализация алгоритма SHA-384 без сторонних библиотек.
 * @author Усачев Тимофей, Корвяков Святогор, Гусейнов Артем
 * @date 2026
 *
 * Реализация соответствует стандарту FIPS PUB 180-4.
 * SHA-384 = SHA-512 с другими начальными значениями H0-H7
 * и усечением результата до первых 384 бит (6 из 8 слов).
 */

#include "sha384.h"
#include <cstring>

// ─────────────────────────────────────────────
//  ТАБЛИЦА КОНСТАНТ K
//
//  80 констант — дробные части кубических корней
//  первых 80 простых чисел, умноженные на 2^64.
//  Определены стандартом SHA-2 (FIPS 180-4).
// ─────────────────────────────────────────────
const uint64_t SHA384::K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
    0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
    0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
    0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
    0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
    0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
    0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
    0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
    0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
    0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
    0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
    0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
    0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
    0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
    0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
    0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
    0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
    0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
    0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
    0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
    0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

// ─────────────────────────────────────────────
//  ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ─────────────────────────────────────────────

uint64_t SHA384::rotr64(uint64_t x, int n)
{
    return (x >> n) | (x << (64 - n));
}

uint64_t SHA384::ch(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (~x & z);
}

uint64_t SHA384::maj(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

uint64_t SHA384::sigma0(uint64_t x)
{
    return rotr64(x, 28) ^ rotr64(x, 34) ^ rotr64(x, 39);
}

uint64_t SHA384::sigma1(uint64_t x)
{
    return rotr64(x, 14) ^ rotr64(x, 18) ^ rotr64(x, 41);
}

uint64_t SHA384::gamma0(uint64_t x)
{
    return rotr64(x, 1) ^ rotr64(x, 8) ^ (x >> 7);
}

uint64_t SHA384::gamma1(uint64_t x)
{
    return rotr64(x, 19) ^ rotr64(x, 61) ^ (x >> 6);
}

// ─────────────────────────────────────────────
//  ОБРАБОТКА ОДНОГО БЛОКА (80 РАУНДОВ)
// ─────────────────────────────────────────────

void SHA384::processBlock(const uint64_t *block, uint64_t h[8])
{
    // расписание сообщения W — расширяем 16 слов блока до 80 слов
    uint64_t W[80];

    // первые 16 слов берём из блока напрямую
    for (int i = 0; i < 16; i++)
        W[i] = block[i];

    // остальные 64 слова вычисляем по формуле расширения
    for (int i = 16; i < 80; i++)
        W[i] = gamma1(W[i-2]) + W[i-7] + gamma0(W[i-15]) + W[i-16];

    // инициализируем рабочие переменные текущими значениями хеша
    uint64_t a = h[0]; 
    uint64_t b = h[1]; 
    uint64_t c = h[2]; 
    uint64_t d = h[3]; 
    uint64_t e = h[4]; 
    uint64_t f = h[5]; 
    uint64_t g = h[6]; 
    uint64_t hh = h[7]; // рабочая переменная h (hh чтобы не конфликтовать с массивом)

    for (int i = 0; i < 80; i++)
    {
        uint64_t T1 = hh + sigma1(e) + ch(e, f, g) + K[i] + W[i];

        uint64_t T2 = sigma0(a) + maj(a, b, c);

        hh = g;      
        g  = f;      
        f  = e;      
        e  = d + T1; 
        d  = c;      
        c  = b;      
        b  = a;      
        a  = T1 + T2; 
    }

    // прибавляем результат раундов к текущему значению хеша
    // сложение по модулю 2^64 (автоматически при переполнении uint64_t)
    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
    h[5] += f;
    h[6] += g;
    h[7] += hh;
}

// ─────────────────────────────────────────────
//  ГЛАВНАЯ ФУНКЦИЯ ХЕШИРОВАНИЯ
// ─────────────────────────────────────────────

QString SHA384::hash(const QString &input)
{
    return hash(input.toUtf8()); // конвертируем строку в байты и хешируем
}

QString SHA384::hash(const QByteArray &data)
{
    // ─── Начальные значения H0-H7 для SHA-384 ───
    // Дробные части квадратных корней 9-16 простых чисел (23,29,31,37,41,43,47,53)
    uint64_t h[8] = {
        0xcbbb9d5dc1059ed8ULL, 
        0x629a292a367cd507ULL, 
        0x9159015a3070dd17ULL, 
        0x152fecd8f70e5939ULL, 
        0x67332667ffc00b31ULL, 
        0x8eb44a8768581511ULL, 
        0xdb0c2e0d64f98fa7ULL, 
        0x47b5481dbefa4fa4ULL  
    };

    // ─── Шаг 1: Подготовка сообщения (padding) ───
    // Нужно дополнить сообщение до длины кратной 1024 битам (128 байтам)

    uint64_t msgLen = data.size();

    // создаём расширенный буфер: сообщение + 1 байт 0x80 + нули + 16 байт длины
    // максимальный размер блока = msgLen + 128 + 128 = достаточно места
    int paddedLen = (int)(((msgLen + 17 + 127) / 128) * 128); // округляем вверх до 128 байт
    QByteArray padded(paddedLen, 0);                    // заполняем нулями

    memcpy(padded.data(), data.constData(), msgLen);

    padded[(int)msgLen] = 0x80; // добавляем бит "1" после сообщения (0x80 = 10000000)

    // записываем длину исходного сообщения в битах в последние 16 байт блока
    // SHA-384 использует 128-битное представление длины (big-endian)
    // мы используем только нижние 64 бита (достаточно для любого реального сообщения)
    uint64_t bitLen = msgLen * 8; // длина в битах

    // записываем в big-endian формате (старший байт первый)
    for (int i = 7; i >= 0; i--)
    {
        padded[paddedLen - 8 + (7 - i)] = (char)((bitLen >> (i * 8)) & 0xFF);
    }

    // ─── Шаг 2: Обработка блоков ───
    // Разбиваем дополненное сообщение на блоки по 128 байт (1024 бит)

    int numBlocks = paddedLen / 128; // количество блоков

    for (int b = 0; b < numBlocks; b++)
    {
        uint64_t block[16]; // блок из 16 64-битных слов

        // читаем 16 слов блока в формате big-endian
        for (int i = 0; i < 16; i++)
        {
            block[i] = 0;

            // каждое 64-битное слово = 8 байт в big-endian
            for (int j = 0; j < 8; j++)
            {
                block[i] = (block[i] << 8) |
                           (uint8_t)padded[(int)(b * 128 + i * 8 + j)];
            }
        }

        processBlock(block, h); // обрабатываем блок — обновляем h[0]-h[7]
    }

    // ─── Шаг 3: Формирование результата ───
    // SHA-384 = первые 6 слов из 8 (384 бита = 6 * 64 бит)

    QString result;
    for (int i = 0; i < 6; i++)
    {
        // конвертируем 64-битное число в 16 hex-символов с ведущими нулями
        result += QString("%1").arg(h[i], 16, 16, QChar('0'));
    }

    return result;
}
