#include "CipherInterface.h"
#include "FunctionsCipher.h"
#include "Cipher.h"

void vigenereValidate(const string& key) {
    validateKey("Vigenere", key); 
}

void vigenereEncryptF(const string& input, const string& output, const string& key) {
    encryptVigenere(input, output, key);
}

void vigenereDecryptF(const string& input, const string& output, const string& key) {
    decryptVigenere(input, output, key);
}

vector<unsigned char> vigenereEncryptC(const string& text, const string& key) {
    return encryptVigenereConsole(text, key);
}

vector<unsigned char> vigenereDecryptC(const string& text, const string& key) {
    return decryptVigenereConsole(text, key);
}

extern "C" {
    CipherPlugin* createCipher() {
        CipherPlugin* cipher = new CipherPlugin;
        cipher->name = "Виженер";
        cipher->validateKey = vigenereValidate;
        cipher->encryptFile = vigenereEncryptF;
        cipher->decryptFile = vigenereDecryptF;
        cipher->encryptConsole = vigenereEncryptC;
        cipher->decryptConsole = vigenereDecryptC;
        return cipher;
    }

    void destroyPlugin(CipherPlugin* p) {
        delete p;
    }
}