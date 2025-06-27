#include "Password.h"
#include <limits> 

using namespace std;

size_t countHash(const string& passwd) {
    size_t hashValue = 0;
    const size_t p = 41;
    const size_t mod = 1e9 + 7;
    
    for (char c : passwd) {
        hashValue = (hashValue * p + c) % mod;
    }
    return hashValue;
}

bool checkPasswd() {
    ifstream ifs("password.txt");
    if (!ifs.is_open()) {
        initPasswd();
        return true;
    }

    size_t storedHash;
    ifs >> storedHash;
    ifs.close();

    cout << "Введите пароль: ";
    string userInput;
    getline(cin, userInput); 

    if (countHash(userInput) == storedHash) {
        return true;
    } else {
        cout << "Неверный пароль" << endl;
        return false;
    }
}

void changePasswd() {
    ifstream ifs("password.txt");
    if (!ifs.is_open()) {
        cout << "Файл пароля не существует. Создайте пароль сначала." << endl;
        initPasswd();
        return;
    }

    size_t curHash;
    ifs >> curHash;
    ifs.close();

    for (int i = 0; i < 3; i++) {
        cout << "Введите текущий пароль: ";
        string userInput;
        getline(cin, userInput); 
        if (curHash == countHash(userInput)) {
            cout << "Введите новый пароль: ";
            getline(cin, userInput); 

            ofstream ofs("password.txt", ios::trunc);
            ofs << countHash(userInput);
            ofs.close();

            cout << "Пароль изменен успешно :)" << endl;
            break;
        } else if (i < 2) {
            cout << "Неправильно. Попробуйте еще раз ~^-^~" << endl;
        } else {
            cout << "Вы отчислены XD" << endl;
        }
    }
}

void initPasswd() {
    cout << "Файл пароля не существует. Введите новый пароль: ";
    string userInput;
    getline(cin, userInput); 
    
    ofstream ofs("password.txt", ios::trunc);
    ofs << countHash(userInput);
    ofs.close();

    cout << "Пароль задан успешно :)" << endl;
}