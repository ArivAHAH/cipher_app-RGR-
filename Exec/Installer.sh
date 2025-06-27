#!/bin/bash

#  Переменные 
# Имя приложения (должно совпадать с TARGET в Makefile)
appName="cipher_app"

# Куда будут установлены основные файлы (бинарник и шифры)
installDirLib="$HOME/${appName}"

# Куда будет установлена символическая ссылка для запуска
installDirBin="/usr/local/bin"

# Имя исполняемого файла и папки с шифрами
appExecutable="cipher_app"
ciphersDir="ciphers"

# Функция для вывода сообщений
logMessage() {
    echo "$1"
}

# Проверка, запущен ли скрипт с правами root (через suSSdo)
checkRoot() {
    if [ "$(id -u)" != "0" ]; then
        logMessage "Ошибка: для установки требуются права администратора."
        logMessage "Пожалуйста, запустите скрипт с помощью 'sudo': sudo make installer"
        exit 1
    fi
}

# Основная функция установки
doInstall() {
    logMessage "Начинаю установку ${appName}..."

    # 1. Проверяем, что все нужные файлы на месте
    if [ ! -f "$appExecutable" ]; then
        logMessage "Ошибка: исполняемый файл '${appExecutable}' не найден. Сначала соберите проект: make"
        exit 1
    fi
    if [ ! -d "$ciphersDir" ]; then
        logMessage "Ошибка: папка с шифрами '${ciphersDir}' не найдена. Сначала соберите проект: make"
        exit 1
    fi

    # 2. Создаем директории для установки
    logMessage "Создаю директории: ${installDirLib}"
    mkdir -p "$installDirLib"
    
    # 3. Копируем файлы
    logMessage "Копирую файлы приложения..."
    cp -r "$ciphersDir" "$installDirLib/"
    cp "$appExecutable" "$installDirLib/"

    # 4. Создаем скрипт-обертку для запуска
    logMessage "Создаю команду для запуска в ${installDirBin}/${appName}"
    # Используем 'tee' для записи с правами sudo, что надежнее
    tee "${installDirBin}/${appName}" > /dev/null << EOF
#!/bin/bash
# Скрипт-обертка для запуска приложения
cd "${installDirLib}" && ./${appExecutable} "\$@"
EOF

    # 5. Делаем скрипт-обертку исполняемым
    chmod +x "${installDirBin}/${appName}"

    # 6. Создаем скрипт для удаления
    logMessage "Создаю скрипт для удаления: ${installDirLib}/uninstall.sh"
    tee "${installDirLib}/uninstall.sh" > /dev/null << EOF
#!/bin/bash
echo "Удаляю ${appName}..."
rm -f "${installDirBin}/${appName}"
rm -rf "${installDirLib}"
echo "Приложение ${appName} успешно удалено."
EOF
    chmod +x "${installDirLib}/uninstall.sh"

    logMessage "--"
    logMessage "Установка завершена успешно!"
    logMessage "Теперь вы можете запустить приложение из любого места, просто введя команду:"
    logMessage "  ${appName}"
    logMessage ""
    logMessage "Для удаления запустите:"
    logMessage "  sudo ${installDirLib}/uninstall.sh"
    logMessage "--"
}

#  Основная логика скрипта 
checkRoot
doInstall

exit 0
