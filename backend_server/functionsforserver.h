#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>

// Главная функция парсинга команд от клиента
QString parsing(QString request);

// Шифр Виженера
QString vigenereEncrypt(const QString &text, const QString &key);
QString vigenereDecrypt(const QString &text, const QString &key);

// Хеш SHA-384
QString sha384(const QString &text);

// Метод хорд (находит корень уравнения x^3 - x - 2 = 0)
double chordMethod(double a, double b, double eps);
QString chordMethodStr(const QString &a, const QString &b, const QString &eps);

// Стеганография (LSB в BMP)
QString stegoEncode(const QString &imagePath, const QString &message);
QString stegoDecode(const QString &imagePath);

#endif // FUNCTIONSFORSERVER_H
