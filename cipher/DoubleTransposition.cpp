#include "CipherInterface.h"
#include "FunctionsCipher.h"
#include "Cipher.h"

void dtValidate(const string& key) {
    validateKey("DoubleTransposition", key); 
}

void dtEncryptF(const string& input, const string& output, const string& key) {
    encryptDoubleTransposition(input, output, key);
}

void dtDecryptF(const string& input, const string& output, const string& key) {
    decryptDoubleTransposition(input, output, key);
}

vector<unsigned char> dtEncryptC(const string& text, const string& key) {
    return encryptDoubleTranspositionConsole(text, key);
}

vector<unsigned char> dtDecryptC(const string& text, const string& key) {
    return decryptDoubleTranspositionConsole(text, key);
}

extern "C" {
    CipherPlugin* createCipher() {
        CipherPlugin* cipher = new CipherPlugin;
        cipher->name = "Двойная табличная перестановка";
        cipher->validateKey = dtValidate;
        cipher->encryptFile = dtEncryptF;
        cipher->decryptFile = dtDecryptF;
        cipher->encryptConsole = dtEncryptC;
        cipher->decryptConsole = dtDecryptC;
        return cipher;
    }

    void destroyPlugin(CipherPlugin* p) {
        delete p;
    }
}