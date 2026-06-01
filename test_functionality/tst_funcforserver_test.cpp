#include <QtTest>
#include "../backend_server/functionsforserver.h"
#include "../backend_server/database.h"

class FuncForServer_Test : public QObject
{
    Q_OBJECT

public:
    FuncForServer_Test();
    ~FuncForServer_Test();

private slots:

    // --- Парсинг ---
    void test_parsing_unknown_command();
    void test_parsing_empty_string();

    // --- Шифр Виженера ---
    void test_vigenere_encrypt_basic();
    void test_vigenere_decrypt_basic();
    void test_vigenere_roundtrip();
    void test_vigenere_empty_key();
    void test_vigenere_lowercase();
    void test_vigenere_via_parsing();

    // --- SHA-384 ---
    void test_sha384_length();
    void test_sha384_known_value();
    void test_sha384_empty_string();
    void test_sha384_via_parsing();

    // --- Метод хорд ---
    void test_chord_known_root();
    void test_chord_bad_interval();
    void test_chord_via_parsing();

    // --- База данных (Singleton) ---
    void test_database_singleton();
    void test_database_register();
    void test_database_login_success();
    void test_database_login_wrong_password();
    void test_database_duplicate_user();
    void test_database_admin_exists();
};

FuncForServer_Test::FuncForServer_Test() {}
FuncForServer_Test::~FuncForServer_Test() {}

// ─────────────────────────────────────────────
//  ПАРСИНГ
// ─────────────────────────────────────────────
void FuncForServer_Test::test_parsing_unknown_command()
{
    QVERIFY2(parsing("ewqeewqewq") == "error",
             "Unknown command should return 'error'");
}

void FuncForServer_Test::test_parsing_empty_string()
{
    QVERIFY2(parsing("") == "error",
             "Empty string should return 'error'");
}

// ─────────────────────────────────────────────
//  ШИФР ВИЖЕНЕРА
// ─────────────────────────────────────────────
void FuncForServer_Test::test_vigenere_encrypt_basic()
{
    QString result = vigenereEncrypt("HELLO", "KEY");
    QVERIFY2(result == "RIJVS",
             qPrintable("Expected RIJVS, got: " + result));
}

void FuncForServer_Test::test_vigenere_decrypt_basic()
{
    QString result = vigenereDecrypt("RIJVS", "KEY");
    QVERIFY2(result == "HELLO",
             qPrintable("Expected HELLO, got: " + result));
}

void FuncForServer_Test::test_vigenere_roundtrip()
{
    QString original  = "ATTACKATDAWN";
    QString key       = "LEMON";
    QString encrypted = vigenereEncrypt(original, key);
    QString decrypted = vigenereDecrypt(encrypted, key);
    QVERIFY2(decrypted == original,
             qPrintable("Roundtrip failed: " + decrypted));
}

void FuncForServer_Test::test_vigenere_empty_key()
{
    QVERIFY2(vigenereEncrypt("HELLO", "") == "error",
             "Empty key should return 'error'");
}

void FuncForServer_Test::test_vigenere_lowercase()
{
    // Регистр должен сохраняться
    QString result = vigenereEncrypt("Hello", "KEY");
    QVERIFY2(result == "Rijvs",
             qPrintable("Expected Rijvs, got: " + result));
}

void FuncForServer_Test::test_vigenere_via_parsing()
{
    QString result = parsing("vigenere_encrypt:HELLO:KEY");
    QVERIFY2(result == "RIJVS",
             qPrintable("parsing vigenere failed: " + result));
}

// ─────────────────────────────────────────────
//  SHA-384
// ─────────────────────────────────────────────
void FuncForServer_Test::test_sha384_length()
{
    QString result = sha384("test");
    QVERIFY2(result.length() == 96,
             "SHA-384 hex should be 96 characters");
}

void FuncForServer_Test::test_sha384_known_value()
{
    // Известный SHA-384 хеш для "abc"
    QString expected = "cb00753f45a35e8bb5a03d699ac65007"
                       "272c32ab0eded1631a8b605a43ff5bed"
                       "8086072ba1e7cc2358baeca134c825a7";
    QString result = sha384("abc");
    QVERIFY2(result == expected,
             qPrintable("SHA-384 mismatch: " + result));
}

void FuncForServer_Test::test_sha384_empty_string()
{
    QString result = sha384("");
    QVERIFY2(result.length() == 96,
             "SHA-384 of empty string should be 96 chars");
}

void FuncForServer_Test::test_sha384_via_parsing()
{
    QString result = parsing("sha384:abc");
    QVERIFY2(result.length() == 96,
             "sha384 via parsing should return 96-char hex");
}

// ─────────────────────────────────────────────
//  МЕТОД ХОРД
// ─────────────────────────────────────────────
void FuncForServer_Test::test_chord_known_root()
{
    // f(x) = x^3 - x - 2 = 0, корень ≈ 1.5213797
    double result = chordMethod(1.0, 2.0, 1e-9);
    QVERIFY2(std::fabs(result - 1.5213797) < 1e-5,
             qPrintable(QString("Root mismatch: %1").arg(result)));
}

void FuncForServer_Test::test_chord_bad_interval()
{
    // f(0)*f(1) > 0 — корня нет на [0,1]
    QString result = chordMethodStr("0", "1", "0.001");
    QVERIFY2(result.contains("error"),
             "Bad interval should return error");
}

void FuncForServer_Test::test_chord_via_parsing()
{
    QString result = parsing("chord:1:2:0.000001");
    bool ok;
    double val = result.toDouble(&ok);
    QVERIFY2(ok && std::fabs(val - 1.5213797) < 1e-4,
             qPrintable("chord via parsing failed: " + result));
}

// ─────────────────────────────────────────────
//  БАЗА ДАННЫХ (SINGLETON)
// ─────────────────────────────────────────────
void FuncForServer_Test::test_database_singleton()
{
    // Два вызова getInstance() должны вернуть один и тот же объект
    Database *db1 = Database::getInstance();
    Database *db2 = Database::getInstance();
    QVERIFY2(db1 == db2,
             "Singleton must return the same instance");
}

void FuncForServer_Test::test_database_register()
{
    QString result = Database::getInstance()
                     ->registerUser("testuser_unit", "testpass123");
    QVERIFY2(result == "ok: registered",
             qPrintable("Register failed: " + result));
}

void FuncForServer_Test::test_database_login_success()
{
    // Сначала регистрируем
    Database::getInstance()->registerUser("testlogin_unit", "mypassword");
    // Потом логинимся
    QString result = Database::getInstance()
                     ->loginUser("testlogin_unit", "mypassword");
    QVERIFY2(result == "user",
             qPrintable("Login failed: " + result));
}

void FuncForServer_Test::test_database_login_wrong_password()
{
    Database::getInstance()->registerUser("wrongpass_unit", "correctpass");
    QString result = Database::getInstance()
                     ->loginUser("wrongpass_unit", "wrongpass");
    QVERIFY2(result.startsWith("error"),
             "Wrong password should return error");
}

void FuncForServer_Test::test_database_duplicate_user()
{
    Database::getInstance()->registerUser("duplicate_unit", "pass");
    // Повторная регистрация того же логина
    QString result = Database::getInstance()
                     ->registerUser("duplicate_unit", "pass");
    QVERIFY2(result.startsWith("error"),
             "Duplicate user should return error");
}

void FuncForServer_Test::test_database_admin_exists()
{
    // Администратор создаётся автоматически при старте
    QString result = Database::getInstance()
                     ->loginUser("admin", "admin123");
    QVERIFY2(result == "admin",
             qPrintable("Admin login failed: " + result));
}

QTEST_MAIN(FuncForServer_Test)
#include "tst_funcforserver_test.moc"
