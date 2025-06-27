#include "Utills.h"
#include <iostream>
#include <sstream>
#include <dlfcn.h>
#include <dirent.h>

using namespace std;

vector<LoadedCipherInfo> gLoadedCiphers;

void loadCiphers(const string& dirPath) {
    cout << "Поиск шифров в папке: " << dirPath << "...\n";
    DIR* dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        cout << "Не могу открыть папку с шифрами!\n";
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        string fileName = entry->d_name;
        if (fileName.find(".so") == string::npos && fileName.find(".dylib") == string::npos) {
            continue;
        }

        string fullPath = dirPath + "/" + fileName;
        void* handle = dlopen(fullPath.c_str(), RTLD_LAZY);
        if (!handle) {
            cout << "Ошибка загрузки шифра " << fileName << ": " << dlerror() << endl;
            continue;
        }

        CipherPlugin* (*createFunc)() = (CipherPlugin* (*)())dlsym(handle, "createCipher");
        void (*destroyFunc)(CipherPlugin*) = (void (*)(CipherPlugin*))dlsym(handle, "destroyPlugin");
        
        if (!createFunc || !destroyFunc) {
            cout << "Шифр " << fileName << " сломан (нет createCipher или destroyPlugin)!\n";
            dlclose(handle);
            continue;
        }
        
        LoadedCipherInfo info;
        info.cipher = createFunc();
        info.handle = handle;
        info.destroyFunc = destroyFunc;
        gLoadedCiphers.push_back(info);

        cout << "  -> Загружен: " << info.cipher->name << endl;
    }
    closedir(dir);
}

void unloadCiphers() {
    cout << "Выгружаю все шифры...\n";
    for (int i = 0; i < gLoadedCiphers.size(); ++i) {
        LoadedCipherInfo info = gLoadedCiphers[i];
        info.destroyFunc(info.cipher);
        dlclose(info.handle);
    }
    gLoadedCiphers.clear();
}

bool isValidNumber(const string& str, int& result) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isdigit(c)) return false;
    }
    stringstream ss(str);
    ss >> result;
    return ss.eof() && !ss.fail();
}

wchar_t customToWUpper(wchar_t c) {
    if (c >= L'а' && c <= L'я') {
        return c - (L'а' - L'А');
    } else if (c == L'ё') {
        return L'Ё';
    }
    if (c >= L'a' && c <= L'z') {
        return toupper(c);
    }
    return c;
}

wchar_t customToWLower(wchar_t c) {
    if (c >= L'А' && c <= L'Я') {
        return c + (L'а' - L'А');
    } else if (c == L'Ё') {
        return L'ё';
    }
    if (c >= L'A' && c <= L'Z') {
        return tolower(c);
    }
    return c;
}

bool isValidUtf8(const vector<unsigned char>& data) {
    size_t i = 0;
    while (i < data.size()) {
        if (data[i] <= 0x7F) {
            i++;
        } else if (i + 1 < data.size() && (data[i] & 0xE0) == 0xC0 && (data[i + 1] & 0xC0) == 0x80) {
            i += 2;
        } else if (i + 2 < data.size() && (data[i] & 0xF0) == 0xE0 && (data[i + 1] & 0xC0) == 0x80 && (data[i + 2] & 0xC0) == 0x80) {
            i += 3;
        } else if (i + 3 < data.size() && (data[i] & 0xF8) == 0xF0 && (data[i + 1] & 0xC0) == 0x80 && (data[i + 2] & 0xC0) == 0x80 && (data[i + 3] & 0xC0) == 0x80) {
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}