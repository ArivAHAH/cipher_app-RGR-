#include "CreateFile.h"
#include "Utills.h"

bool createInputFile(const string& filePath) {
    cout << "Файл " << filePath << " не существует. Хотите создать его? (да/нет): ";
    string createFile;
    getline(cin, createFile);
    if (createFile == "да" || createFile == "Да" || createFile == "y" || createFile == "Y") {
        cout << "Введите текст (закончите ввод пустой строкой):\n";
        string content, line;
        while (getline(cin, line) && !line.empty()) {
            content += line + "\n";
        }
        try {
            ofstream newFile(filePath, ios::binary);
            if (!newFile.is_open()) {
                throw runtime_error("Не удалось создать файл: " + filePath);
            }
            newFile.write(content.c_str(), content.size());
            newFile.close();
            cout << "Файл " << filePath << " создан успешно.\n";
            return true;
        } catch (const exception& e) {
            throw runtime_error("Ошибка при создании файла: " + string(e.what()));
        }
    }
    return false;
}