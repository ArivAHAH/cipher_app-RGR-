#include "FunctionsCipher.h"
#include "Utills.h"
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <codecvt>
#include <locale>
#include <cstdint>

using namespace std;

vector<unsigned char> processGronsfeld(const vector<unsigned char>& data, const string& key, bool encrypt, bool isText) {
    if (!isText) {
        vector<unsigned char> result = data;
        size_t keyLength = key.length();
        size_t keyIndex = 0;

        for (size_t i = 0; i < data.size(); ++i) {
            int shift = (key[keyIndex % keyLength] - '0');
            if (shift < 0 || shift > 9) {
                throw invalid_argument("Ключ для Гронсфельда должен содержать только цифры.");
            }

            if (encrypt) {
                result[i] = (data[i] + shift) % 256;
            } else {
                result[i] = (data[i] - shift + 256) % 256;
            }
            keyIndex++;
        }
        return result;
    } else {
        wstring wData, wKey;
        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            wData = converter.from_bytes(string(data.begin(), data.end()));
            wKey = converter.from_bytes(key);
        } catch (const exception&) {
            throw runtime_error("Ошибка преобразования данных или ключа в Unicode.");
        }

        wstring result = wData;
        size_t keyLength = wKey.length();
        size_t keyIndex = 0;

        for (size_t i = 0; i < wData.size(); ++i) {
            wchar_t c = wData[i];
            int shift = (wKey[keyIndex % keyLength] - L'0');
            if (shift < 0 || shift > 9) {
                throw invalid_argument("Ключ для Гронсфельда должен содержать только цифры.");
            }

            bool isLatinUpper = (c >= L'A' && c <= L'Z');
            bool isLatinLower = (c >= L'a' && c <= L'z');
            bool isCyrillicUpper = (c >= L'А' && c <= L'Я') || c == L'Ё';
            bool isCyrillicLower = (c >= L'а' && c <= L'я') || c == L'ё';
            bool isSpace = (c == L' ');

            if (!isLatinUpper && !isLatinLower && !isCyrillicUpper && !isCyrillicLower && !isSpace) {
                continue;
            }

            int charValue, alphabetSize;
            wchar_t baseChar;

            if (isLatinUpper) {
                charValue = c - L'A';
                baseChar = L'A';
                alphabetSize = 26;
            } else if (isLatinLower) {
                charValue = c - L'a';
                baseChar = L'a';
                alphabetSize = 26;
            } else if (isCyrillicUpper) {
                if (c == L'Ё') charValue = 32;
                else charValue = c - L'А';
                baseChar = L'А';
                alphabetSize = 33;
            } else if (isCyrillicLower) {
                if (c == L'ё') charValue = 32;
                else charValue = c - L'а';
                baseChar = L'а';
                alphabetSize = 33;
            } else if (isSpace) {
                result[i] = L' ';
                keyIndex++;
                continue;
            }

            int newValue;
            if (encrypt) {
                newValue = (charValue + shift) % alphabetSize;
            } else {
                newValue = (charValue - shift + alphabetSize) % alphabetSize;
            }

            if (isCyrillicUpper) {
                if (newValue == 32) result[i] = L'Ё';
                else result[i] = baseChar + newValue;
            } else if (isCyrillicLower) {
                if (newValue == 32) result[i] = L'ё';
                else result[i] = baseChar + newValue;
            } else {
                result[i] = baseChar + newValue;
            }

            keyIndex++;
        }

        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            string resultStr = converter.to_bytes(result);
            return vector<unsigned char>(resultStr.begin(), resultStr.end());
        } catch (const exception&) {
            throw runtime_error("Ошибка преобразования результата в UTF-8.");
        }
    }
}

vector<unsigned char> processDoubleTransposition(const vector<unsigned char>& data, const string& key, bool encrypt, bool isText, uint64_t originalSize) {
    stringstream ss(key);
    string colKey, rowKey;
    if (!(ss >> colKey >> rowKey) || !ss.eof()) {
        throw invalid_argument("Ключ должен содержать две части, разделенные пробелом.");
    }

    size_t numCols = colKey.length();
    size_t numRows = rowKey.length();
    size_t blockSize = numRows * numCols;

    vector<int> colPerm(numCols), rowPerm(numRows);
    vector<int> invColPerm(numCols), invRowPerm(numRows);
    for (size_t i = 0; i < numCols; ++i) {
        colPerm[i] = (colKey[i] - '0') - 1;
        invColPerm[colPerm[i]] = i;
    }
    for (size_t i = 0; i < numRows; ++i) {
        rowPerm[i] = (rowKey[i] - '0') - 1;
        invRowPerm[rowPerm[i]] = i;
    }

    if (isText) {
        wstring wData;
        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            wData = converter.from_bytes(string(data.begin(), data.end()));
        } catch (const exception&) {
            throw runtime_error("Ошибка преобразования данных в Unicode.");
        }

        wstring result;
        result.reserve(wData.size());

        for (size_t blockStart = 0; blockStart < wData.size(); blockStart += blockSize) {
            size_t blockEnd = min(blockStart + blockSize, wData.size());
            size_t originalBlockSize = blockEnd - blockStart;

            wstring block = wData.substr(blockStart, originalBlockSize);
            if (encrypt && block.size() < blockSize) {
                block.append(blockSize - block.size(), L' ');
            }

            vector<vector<wchar_t>> table(numRows, vector<wchar_t>(numCols, L' '));
            size_t idx = 0;
            for (size_t i = 0; i < numRows; ++i) {
                for (size_t j = 0; j < numCols; ++j) {
                    if (idx < block.size()) {
                        table[i][j] = block[idx++];
                    }
                }
            }

            if (encrypt) {
                vector<vector<wchar_t>> tempTable(numRows, vector<wchar_t>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[i][colPerm[j]];
                    }
                }
                table = tempTable;

                tempTable = vector<vector<wchar_t>>(numRows, vector<wchar_t>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[rowPerm[i]][j];
                    }
                }
                table = tempTable;
            } else {
                vector<vector<wchar_t>> tempTable(numRows, vector<wchar_t>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[invRowPerm[i]][j];
                    }
                }
                table = tempTable;

                tempTable = vector<vector<wchar_t>>(numRows, vector<wchar_t>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[i][invColPerm[j]];
                    }
                }
                table = tempTable;
            }

            idx = 0;
            size_t charsToTake = encrypt ? blockSize : originalSize > 0 ? min(originalBlockSize, static_cast<size_t>(originalSize - result.size())) : originalBlockSize;
            for (size_t i = 0; i < numRows && idx < charsToTake; ++i) {
                for (size_t j = 0; j < numCols && idx < charsToTake; ++j) {
                    result.push_back(table[i][j]);
                    idx++;
                }
            }
        }

        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            string resultStr = converter.to_bytes(result);
            if (!encrypt && originalSize > 0 && resultStr.size() > originalSize) {
                resultStr.resize(originalSize);
            }
            return vector<unsigned char>(resultStr.begin(), resultStr.end());
        } catch (const exception&) {
            throw runtime_error("Ошибка преобразования результата в UTF-8.");
        }
    } else {
        vector<unsigned char> result;
        result.reserve(data.size());

        for (size_t blockStart = 0; blockStart < data.size(); blockStart += blockSize) {
            size_t blockEnd = min(blockStart + blockSize, data.size());
            size_t currentBlockSize = blockEnd - blockStart;

            vector<unsigned char> block(data.begin() + blockStart, data.begin() + blockEnd);
            if (encrypt && block.size() < blockSize) {
                block.resize(blockSize, 0);
            }

            vector<vector<unsigned char>> table(numRows, vector<unsigned char>(numCols, 0));
            size_t idx = 0;
            for (size_t i = 0; i < numRows && idx < block.size(); ++i) {
                for (size_t j = 0; j < numCols && idx < block.size(); ++j) {
                    table[i][j] = block[idx++];
                }
            }

            if (encrypt) {
                vector<vector<unsigned char>> tempTable(numRows, vector<unsigned char>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[i][colPerm[j]];
                    }
                }
                table = tempTable;

                tempTable = vector<vector<unsigned char>>(numRows, vector<unsigned char>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[rowPerm[i]][j];
                    }
                }
                table = tempTable;
            } else {
                vector<vector<unsigned char>> tempTable(numRows, vector<unsigned char>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[invRowPerm[i]][j];
                    }
                }
                table = tempTable;

                tempTable = vector<vector<unsigned char>>(numRows, vector<unsigned char>(numCols));
                for (size_t i = 0; i < numRows; ++i) {
                    for (size_t j = 0; j < numCols; ++j) {
                        tempTable[i][j] = table[i][invColPerm[j]];
                    }
                }
                table = tempTable;
            }

            idx = 0;
            size_t charsToTake = encrypt ? blockSize : currentBlockSize;
            for (size_t i = 0; i < numRows && idx < charsToTake; ++i) {
                for (size_t j = 0; j < numCols && idx < charsToTake; ++j) {
                    result.push_back(table[i][j]);
                    idx++;
                }
            }
        }

        if (!encrypt && originalSize > 0 && result.size() > originalSize) {
            result.resize(originalSize);
        }

        return result;
    }
}

vector<unsigned char> processVigenere(const vector<unsigned char>& data, const string& key, bool encrypt, bool isText) {
    if (!isText) {
        vector<unsigned char> result = data;
        wstring wKey;
        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            wKey = converter.from_bytes(key);
        } catch (const exception&) {
            throw invalid_argument("Некорректный ключ для Виженера.");
        }
        vector<int> shifts;
        for (wchar_t c : wKey) {
            wchar_t cUpper = customToWUpper(c);
            if (cUpper >= L'A' && cUpper <= L'Z') {
                shifts.push_back(cUpper - L'A');
            } else if (cUpper == L'Ё') {
                shifts.push_back(5);
            } else if (cUpper >= L'А' && cUpper <= L'Я') {
                int idx = cUpper - L'А';
                if (idx >= 5) idx++;
                shifts.push_back(idx);
            } else {
                throw invalid_argument("Ключ содержит недопустимые символы.");
            }
        }
        if (shifts.empty()) {
            throw invalid_argument("Ключ не содержит допустимых символов для шифрования.");
        }
        size_t keyLength = shifts.size();
        size_t keyIndex = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            int shift = shifts[keyIndex % keyLength];
            if (encrypt) {
                result[i] = (data[i] + shift) % 256;
            } else {
                result[i] = (data[i] - shift + 256) % 256;
            }
            keyIndex++;
        }
        return result;
    } else {
        wstring wData, wKey;
        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            wData = converter.from_bytes(string(data.begin(), data.end()));
            wKey = converter.from_bytes(key);
        } catch (const exception&) {
            throw runtime_error("Ошибка преобразования данных или ключа в Unicode.");
        }

        wstring keyUpper;
        keyUpper.reserve(wKey.size());
        for (wchar_t kc : wKey) {
            keyUpper.push_back(customToWUpper(kc));
        }

        vector<int> rusShifts, engShifts;
        rusShifts.reserve(keyUpper.size());
        engShifts.reserve(keyUpper.size());
        for (wchar_t kc : keyUpper) {
            if (kc == L'Ё') {
                rusShifts.push_back(5);
            } else if (kc >= L'А' && kc <= L'Я') {
                int idx = kc - L'А';
                if (idx >= 5) idx++;
                rusShifts.push_back(idx);
            } else if (kc >= L'A' && kc <= L'Z') {
                engShifts.push_back(kc - L'A');
            }
        }

        bool isKeyRussian = !rusShifts.empty();
        bool isKeyEnglish = !engShifts.empty();
        if (!isKeyRussian && !isKeyEnglish) {
            throw invalid_argument("Ключ не содержит допустимых букв для шифрования.");
        }

        size_t rusIdx = 0, engIdx = 0;
        size_t rusKeyLen = rusShifts.size();
        size_t engKeyLen = engShifts.size();
        wstring result = wData;

        for (size_t i = 0; i < wData.size(); ++i) {
            wchar_t c = wData[i];
            bool isRus = (c == L'Ё' || c == L'ё' || (c >= L'А' && c <= L'Я') || (c >= L'а' && c <= L'я'));
            if (isRus) {
                if (!isKeyRussian) {
                    result[i] = c;
                    continue;
                }
                bool origIsUpper = (c == customToWUpper(c));
                wchar_t cUpper = customToWUpper(c);
                int charIdx = (cUpper == L'Ё') ? 5 : cUpper - L'А';
                if (cUpper != L'Ё' && charIdx >= 5) charIdx++;
                int shift = rusShifts[rusIdx % rusKeyLen];
                rusIdx++;
                const int ALPHA_SIZE_RU = 33;
                int newIdx = encrypt ? (charIdx + shift) % ALPHA_SIZE_RU : (charIdx - shift + ALPHA_SIZE_RU) % ALPHA_SIZE_RU;
                wchar_t newChar;
                if (newIdx == 5) {
                    newChar = L'Ё';
                } else if (newIdx < 5) {
                    newChar = wchar_t(L'А' + newIdx);
                } else {
                    newChar = wchar_t(L'А' + (newIdx - 1));
                }
                result[i] = origIsUpper ? newChar : customToWLower(newChar);
                continue;
            }

            bool isEng = (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z');
            if (isEng) {
                if (!isKeyEnglish) {
                    result[i] = c;
                    continue;
                }
                bool origIsUpper = (c >= L'A' && c <= L'Z');
                wchar_t cUpper = customToWUpper(c);
                int charIdx = cUpper - L'A';
                int shift = engShifts[engIdx % engKeyLen];
                engIdx++;
                const int ALPHA_SIZE_EN = 26;
                int newIdx = encrypt ? (charIdx + shift) % ALPHA_SIZE_EN : (charIdx - shift + ALPHA_SIZE_EN) % ALPHA_SIZE_EN;
                wchar_t newChar = wchar_t(L'A' + newIdx);
                result[i] = origIsUpper ? newChar : customToWLower(newChar);
                continue;
            }

            result[i] = c;
        }

        try {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            string resultStr = converter.to_bytes(result);
            return vector<unsigned char>(resultStr.begin(), resultStr.end());
        } catch (const exception&) {
            throw runtime_error("Ошибка преобразования результата в UTF-8.");
        }
    }
}