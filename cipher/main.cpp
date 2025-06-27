#include "Password.h"
#include "CreateFile.h"
#include "Utills.h"
#include <iostream>

using namespace std;

enum MainMenuAction {
    ENCRYPT_CONSOLE = 1, DECRYPT_CONSOLE = 2,
    ENCRYPT_FILE = 3, DECRYPT_FILE = 4,
    CHANGE_PASSWORD = 5, EXIT = 6
};

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    loadCiphers("./ciphers"); 

    if (gLoadedCiphers.empty()) {
        cout << "В папке ./ciphers не найдено ни одного рабочего шифра. Выход.\n";
        return 1;
    }

    if (!checkPasswd()) {
        unloadCiphers(); 
        return 1;
    }

    string inputFilePath, outputFilePath, key, inputText;
    int mainChoice, cipherChoice;

    while (true) {
        try {
            cout << "\nВыберите действие:\n"
                 << "1. Зашифровать текст в консоли\n"
                 << "2. Расшифровать текст с консоли\n"
                 << "3. Зашифровать файл\n"
                 << "4. Расшифровать файл\n"
                 << "5. Сменить пароль\n"
                 << "6. Выйти\n"
                 << "Введите номер (1-6): ";
            
            string input;
            getline(cin, input);
            if (!isValidNumber(input, mainChoice)) {
                throw invalid_argument("Ошибка: Введите целое число от 1 до 6.");
            }

            MainMenuAction action = static_cast<MainMenuAction>(mainChoice);

            if (action == EXIT) break;

            if (action == CHANGE_PASSWORD) {
                changePasswd();
                continue;
            }

            if (action == ENCRYPT_FILE || action == DECRYPT_FILE) {
                cout << "Введите путь к входному файлу: ";
                getline(cin, inputFilePath);
                ifstream inputCheck(inputFilePath, ios::binary);
                if (!inputCheck.is_open()) {
                    if (!createInputFile(inputFilePath)) {
                        throw runtime_error("Входной файл не существует, и создание отклонено.");
                    }
                }
                inputCheck.close();
                cout << "Введите путь к выходному файлу: ";
                getline(cin, outputFilePath);
            } else {
                 cout << "Введите текст для обработки (закончите ввод пустой строкой):\n";
                string line;
                inputText.clear();
                while (getline(cin, line) && !line.empty()) {
                    inputText += line + "\n";
                }
                if (inputText.empty()) {
                    throw runtime_error("Введенный текст пуст.");
                }
            }
            
            cout << "\nВыберите алгоритм шифрования:\n";
            for (size_t i = 0; i < gLoadedCiphers.size(); ++i) {
                cout << i + 1 << ". " << gLoadedCiphers[i].cipher->name << "\n";
            }
            cout << "Введите номер: ";

            getline(cin, input);
            if (!isValidNumber(input, cipherChoice) || cipherChoice < 1 || cipherChoice > gLoadedCiphers.size()) {
                throw invalid_argument("Ошибка: Введите корректный номер из списка.");
            }
            
            // Получаем указатель на выбранный шифр из глобального вектора
            CipherPlugin* selectedCipher = gLoadedCiphers[cipherChoice - 1].cipher;
            
            cout << "\nВведите ключ для '" << selectedCipher->name << "': ";
            getline(cin, key);
            
            // Вызываем функции через указатели
            selectedCipher->validateKey(key); 
            
            vector<unsigned char> result;
            switch (action) {
                case ENCRYPT_CONSOLE:
                    result = selectedCipher->encryptConsole(inputText, key);
                    cout << "Результат шифрования:\n" << string(result.begin(), result.end()) << endl;
                    break;
                case DECRYPT_CONSOLE:
                    result = selectedCipher->decryptConsole(inputText, key);
                    cout << "Результат расшифрования:\n" << string(result.begin(), result.end()) << endl;
                    break;
                case ENCRYPT_FILE:
                    selectedCipher->encryptFile(inputFilePath, outputFilePath, key);
                    cout << "Шифрование файла завершено: " << outputFilePath << "\n";
                    break;
                case DECRYPT_FILE:
                    selectedCipher->decryptFile(inputFilePath, outputFilePath, key);
                    cout << "Расшифрование файла завершено: " << outputFilePath << "\n";
                    break;
                default: break;
            }
        } catch (const exception& e) {
            cout << "Ошибка. " << e.what() << "\n";
            cin.clear();
        }
    }

    unloadCiphers(); // Выгружаем шифры перед выходом
    return 0;
}