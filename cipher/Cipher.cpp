#include "Cipher.h"
#include "FunctionsCipher.h"
#include "Utills.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <codecvt>
#include <locale>
#include <cstdint>

using namespace std;

void validateKey(const string& cipherType, const string& key) {
    if (key.empty()) {
        throw invalid_argument("Ключ не может быть пустым.");
    }

    if (cipherType == "Gronsfeld") {
        for (char c : key) {
            if (!isdigit(c)) {
                throw invalid_argument("Ключ для Гронсфельда должен содержать только цифры.");
            }
        }
    } else if (cipherType == "DoubleTransposition") {
        stringstream ss(key);
        string colKey, rowKey;
        if (!(ss >> colKey >> rowKey) || !ss.eof()) {
            throw invalid_argument("Ключ для двойной перестановки должен содержать две части, разделенные пробелом (например, '312 23415').");
        }

        if (colKey.empty() || rowKey.empty()) {
            throw invalid_argument("Обе части ключа (для столбцов и строк) должны быть непустыми.");
        }

        size_t numCols = colKey.length();
        size_t numRows = rowKey.length();

        for (char c : colKey) {
            if (!isdigit(c)) {
                throw invalid_argument("Ключ для столбцов должен содержать только цифры.");
            }
        }
        for (char c : rowKey) {
            if (!isdigit(c)) {
                throw invalid_argument("Ключ для строк должен содержать только цифры.");
            }
        }

        vector<int> colPerm(numCols), rowPerm(numRows);
        for (size_t i = 0; i < numCols; ++i) {
            colPerm[i] = colKey[i] - '0';
            if (colPerm[i] < 1 || colPerm[i] > numCols) {
                throw invalid_argument("Цифры в ключе для столбцов должны быть в диапазоне от 1 до " + to_string(numCols) + ".");
            }
        }
        for (size_t i = 0; i < numRows; ++i) {
            rowPerm[i] = rowKey[i] - '0';
            if (rowPerm[i] < 1 || colPerm[i] > numRows) {
                throw invalid_argument("Цифры в ключе для строк должны быть в диапазоне от 1 до " + to_string(numRows) + ".");
            }
        }

        vector<int> sortedColPerm = colPerm;
        vector<int> sortedRowPerm = rowPerm;
        sort(sortedColPerm.begin(), sortedColPerm.end());
        sort(sortedRowPerm.begin(), sortedRowPerm.end());
        for (size_t i = 0; i < numCols; ++i) {
            if (sortedColPerm[i] != i + 1) {
                throw invalid_argument("Ключ для столбцов должен содержать уникальные цифры от 1 до " + to_string(numCols) + ".");
            }
        }
        for (size_t i = 0; i < numRows; ++i) {
            if (sortedRowPerm[i] != i + 1) {
                throw invalid_argument("Ключ для строк должен содержать уникальные цифры от 1 до " + to_string(numRows) + ".");
            }
        }
    } else if (cipherType == "Vigenere") {
        wstring wKey;
        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            wKey = converter.from_bytes(key);
        } catch (const exception&) {
            throw invalid_argument("Ключ для Виженера содержит некорректные символы.");
        }
        bool hasLatin = false, hasCyrillic = false;
        for (wchar_t c : wKey) {
            if ((c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z')) {
                hasLatin = true;
            } else if ((c >= L'А' && c <= L'Я') || (c >= L'а' && c <= L'я') || c == L'Ё' || c == L'ё') {
                hasCyrillic = true;
            } else {
                throw invalid_argument("Ключ для Виженера должен содержать только латинские или кириллические буквы.");
            }
        }
        if (hasLatin && hasCyrillic) {
            throw invalid_argument("Ключ для Виженера не может содержать одновременно латинские и кириллические буквы.");
        }
        if (!hasLatin && !hasCyrillic) {
            throw invalid_argument("Ключ для Виженера не содержит допустимых букв.");
        }
    } else {
        throw invalid_argument("Неизвестный тип шифра: " + cipherType);
    }
}

void encryptGronsfeld(const string& inputFilePath, const string& outputFilePath, const string& key) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Не удалось открыть входной файл: " + inputFilePath);
    }

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        throw runtime_error("Не удалось открыть выходной файл: " + outputFilePath);
    }

    vector<unsigned char> data((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    if (data.empty()) {
        throw runtime_error("Входной файл пуст.");
    }

    try {
        bool isText = isValidUtf8(data);
        vector<unsigned char> encryptedData = processGronsfeld(data, key, true, isText);
        outputFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
        outputFile.close();
    } catch (const exception& e) {
        outputFile.close();
        throw runtime_error("Ошибка шифрования (Гронсфельд): " + string(e.what()));
    }
}

void decryptGronsfeld(const string& inputFilePath, const string& outputFilePath, const string& key) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Не удалось открыть входной файл: " + inputFilePath);
    }

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        throw runtime_error("Не удалось открыть выходной файл: " + outputFilePath);
    }

    vector<unsigned char> data((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    if (data.empty()) {
        throw runtime_error("Входной файл пуст.");
    }

    try {
        bool isText = isValidUtf8(data);
        vector<unsigned char> decryptedData = processGronsfeld(data, key, false, isText);
        outputFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
        outputFile.close();
    } catch (const exception& e) {
        outputFile.close();
        throw runtime_error("Ошибка дешифрования (Гронсфельд): " + string(e.what()));
    }
}

void encryptDoubleTransposition(const string& inputFilePath, const string& outputFilePath, const string& key) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Не удалось открыть входной файл: " + inputFilePath);
    }

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        throw runtime_error("Не удалось открыть выходной файл: " + outputFilePath);
    }

    vector<unsigned char> data((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    if (data.empty()) {
        throw runtime_error("Входной файл пуст.");
    }

    try {
        bool isText = isValidUtf8(data);
        uint64_t dataSize = isText ? string(data.begin(), data.end()).size() : data.size();
        outputFile.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        vector<unsigned char> encryptedData = processDoubleTransposition(data, key, true, isText);
        outputFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
        outputFile.close();
    } catch (const exception& e) {
        outputFile.close();
        throw runtime_error("Ошибка шифрования (двойная перестановка): " + string(e.what()));
    }
}

void decryptDoubleTransposition(const string& inputFilePath, const string& outputFilePath, const string& key) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Не удалось открыть входной файл: " + inputFilePath);
    }

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        throw runtime_error("Не удалось открыть выходной файл: " + outputFilePath);
    }

    uint64_t originalSize;
    inputFile.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
    if (inputFile.gcount() != sizeof(originalSize)) {
        inputFile.close();
        outputFile.close();
        throw runtime_error("Некорректный формат зашифрованного файла: не удалось прочитать размер данных.");
    }

    vector<unsigned char> data((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    if (data.empty()) {
        throw runtime_error("Входной файл пуст (после заголовка).");
    }

    try {
        bool isText = isValidUtf8(data);
        vector<unsigned char> decryptedData = processDoubleTransposition(data, key, false, isText, originalSize);
        outputFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
        outputFile.close();
    } catch (const exception& e) {
        outputFile.close();
        throw runtime_error("Ошибка дешифрования (двойная перестановка): " + string(e.what()));
    }
}

void encryptVigenere(const string& inputFilePath, const string& outputFilePath, const string& key) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Не удалось открыть входной файл: " + inputFilePath);
    }

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        throw runtime_error("Не удалось открыть выходной файл: " + outputFilePath);
    }

    vector<unsigned char> data((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    if (data.empty()) {
        throw runtime_error("Входной файл пуст.");
    }

    try {
        bool isText = isValidUtf8(data);
        vector<unsigned char> encryptedData = processVigenere(data, key, true, isText);
        outputFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
        outputFile.close();
    } catch (const exception& e) {
        outputFile.close();
        throw runtime_error("Ошибка шифрования (Виженер): " + string(e.what()));
    }
}

void decryptVigenere(const string& inputFilePath, const string& outputFilePath, const string& key) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Не удалось открыть входной файл: " + inputFilePath);
    }

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        throw runtime_error("Не удалось открыть выходной файл: " + outputFilePath);
    }

    vector<unsigned char> data((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    if (data.empty()) {
        throw runtime_error("Входной файл пуст.");
    }

    try {
        bool isText = isValidUtf8(data);
        vector<unsigned char> decryptedData = processVigenere(data, key, false, isText);
        outputFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
        outputFile.close();
    } catch (const exception& e) {
        outputFile.close();
        throw runtime_error("Ошибка дешифрования (Виженер): " + string(e.what()));
    }
}

vector<unsigned char> encryptGronsfeldConsole(const string& inputText, const string& key) {
    if (inputText.empty()) {
        throw runtime_error("Введенный текст пуст.");
    }

    try {
        vector<unsigned char> data(inputText.begin(), inputText.end());
        return processGronsfeld(data, key, true, true);
    } catch (const exception& e) {
        throw runtime_error("Ошибка шифрования (Гронсфельд): " + string(e.what()));
    }
}

vector<unsigned char> decryptGronsfeldConsole(const string& inputText, const string& key) {
    if (inputText.empty()) {
        throw runtime_error("Введенный текст пуст.");
    }

    try {
        vector<unsigned char> data(inputText.begin(), inputText.end());
        return processGronsfeld(data, key, false, true);
    } catch (const exception& e) {
        throw runtime_error("Ошибка дешифрования (Гронсфельд): " + string(e.what()));
    }
}

vector<unsigned char> encryptDoubleTranspositionConsole(const string& inputText, const string& key) {
    if (inputText.empty()) {
        throw runtime_error("Введенный текст пуст.");
    }

    try {
        vector<unsigned char> data(inputText.begin(), inputText.end());
        return processDoubleTransposition(data, key, true, true, data.size());
    } catch (const exception& e) {
        throw runtime_error("Ошибка шифрования (двойная перестановка): " + string(e.what()));
    }
}

vector<unsigned char> decryptDoubleTranspositionConsole(const string& inputText, const string& key) {
    if (inputText.empty()) {
        throw runtime_error("Введенный текст пуст.");
    }

    try {
        vector<unsigned char> data(inputText.begin(), inputText.end());
        return processDoubleTransposition(data, key, false, true, data.size());
    } catch (const exception& e) {
        throw runtime_error("Ошибка дешифрования (двойная перестановка): " + string(e.what()));
    }
}

vector<unsigned char> encryptVigenereConsole(const string& inputText, const string& key) {
    if (inputText.empty()) {
        throw runtime_error("Введенный текст пуст.");
    }

    try {
        vector<unsigned char> data(inputText.begin(), inputText.end());
        return processVigenere(data, key, true, true);
    } catch (const exception& e) {
        throw runtime_error("Ошибка шифрования (Виженер): " + string(e.what()));
    }
}

vector<unsigned char> decryptVigenereConsole(const string& inputText, const string& key) {
    if (inputText.empty()) {
        throw runtime_error("Введенный текст пуст.");
    }

    try {
        vector<unsigned char> data(inputText.begin(), inputText.end());
        return processVigenere(data, key, false, true);
    } catch (const exception& e) {
        throw runtime_error("Ошибка дешифрования (Виженер): " + string(e.what()));
    }
}