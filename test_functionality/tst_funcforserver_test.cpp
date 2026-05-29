#include <QtTest>
#include "../backend_server/functionsforserver.h"

class FuncForServer_Test : public QObject
{
    Q_OBJECT

public:
    FuncForServer_Test();
    ~FuncForServer_Test();

private slots:

    // --- Общий парсинг ---
    void test_parsing_unknown_command();
    void test_parsing_empty_string();

    // --- Шифр Виженера ---
    void test_vigenere_encrypt_basic();
    void test_vigenere_decrypt_basic();
    void test_vigenere_encrypt_decrypt_roundtrip();
    void test_vigenere_empty_key();

    // --- SHA-384 ---
    void test_sha384_known_hash();
    void test_sha384_empty_string();

    // --- Метод хорд ---
    void test_chord_known_root();
    void test_chord_bad_interval();

    // --- Парсинг команд ---
    void test_parsing_vigenere_encrypt();
    void test_parsing_sha384();
    void test_parsing_chord();
};

FuncForServer_Test::FuncForServer_Test() {}
FuncForServer_Test::~FuncForServer_Test() {}

// ─────────────────────────────────────────────
//  Общий парсинг
// ─────────────────────────────────────────────
void FuncForServer_Test::test_parsing_unknown_command()
{
    QVERIFY2(parsing("ewqeewqewq") == "error", "Unknown command should return 'error'");
}

void FuncForServer_Test::test_parsing_empty_string()
{
    QVERIFY2(parsing("") == "error", "Empty string should return 'error'");
}

// ─────────────────────────────────────────────
//  Шифр Виженера
// ─────────────────────────────────────────────
void FuncForServer_Test::test_vigenere_encrypt_basic()
{
    // HELLO зашифрованное ключом KEY = RIJVS
    QString result = vigenereEncrypt("HELLO", "KEY");
    QVERIFY2(result == "RIJVS", qPrintable("Expected RIJVS, got: " + result));
}

void FuncForServer_Test::test_vigenere_decrypt_basic()
{
    QString result = vigenereDecrypt("RIJVS", "KEY");
    QVERIFY2(result == "HELLO", qPrintable("Expected HELLO, got: " + result));
}

void FuncForServer_Test::test_vigenere_encrypt_decrypt_roundtrip()
{
    QString original = "ATTACK";
    QString key = "LEMON";
    QString encrypted = vigenereEncrypt(original, key);
    QString decrypted = vigenereDecrypt(encrypted, key);
    QVERIFY2(decrypted == original, qPrintable("Roundtrip failed: " + decrypted));
}

void FuncForServer_Test::test_vigenere_empty_key()
{
    QString result = vigenereEncrypt("HELLO", "");
    QVERIFY2(result == "error", "Empty key should return 'error'");
}

// ─────────────────────────────────────────────
//  SHA-384
// ─────────────────────────────────────────────
void FuncForServer_Test::test_sha384_known_hash()
{
    // Известный SHA-384 хеш для строки "abc"
    QString expected = "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7";
    QString result = sha384("abc");
    QVERIFY2(result == expected, qPrintable("SHA-384 mismatch for 'abc': " + result));
}

void FuncForServer_Test::test_sha384_empty_string()
{
    // SHA-384 пустой строки — известное значение
    QString result = sha384("");
    QVERIFY2(result.length() == 96, "SHA-384 hex should be 96 characters long");
}

// ─────────────────────────────────────────────
//  Метод хорд
// ─────────────────────────────────────────────
void FuncForServer_Test::test_chord_known_root()
{
    // Уравнение x^3 - x - 2 = 0, корень ≈ 1.5213797
    double result = chordMethod(1.0, 2.0, 1e-9);
    QVERIFY2(std::fabs(result - 1.5213797) < 1e-5,
             qPrintable(QString("Root mismatch: %1").arg(result)));
}

void FuncForServer_Test::test_chord_bad_interval()
{
    // Оба конца одного знака — корня нет
    QString result = chordMethodStr("0", "1", "0.001");
    QVERIFY2(result.contains("error"), "Should return error for bad interval");
}

// ─────────────────────────────────────────────
//  Парсинг команд через parsing()
// ─────────────────────────────────────────────
void FuncForServer_Test::test_parsing_vigenere_encrypt()
{
    QString result = parsing("vigenere_encrypt:HELLO:KEY");
    QVERIFY2(result == "RIJVS", qPrintable("parsing vigenere_encrypt failed: " + result));
}

void FuncForServer_Test::test_parsing_sha384()
{
    QString result = parsing("sha384:abc");
    QVERIFY2(result.length() == 96, "SHA-384 via parsing should return 96-char hex");
}

void FuncForServer_Test::test_parsing_chord()
{
    QString result = parsing("chord:1:2:0.000001");
    bool ok;
    double val = result.toDouble(&ok);
    QVERIFY2(ok && std::fabs(val - 1.5213797) < 1e-4,
             qPrintable("chord via parsing failed: " + result));
}

QTEST_APPLESS_MAIN(FuncForServer_Test)
#include "tst_funcforserver_test.moc"
