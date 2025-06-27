#include "CipherInterface.h"
#include "FunctionsCipher.h"
#include "Cipher.h"

void gronsfeldValidate(const string& key) {
    validateKey("Gronsfeld", key); 
}

void gronsfeldEncryptF(const string& input, const string& output, const string& key) {
    encryptGronsfeld(input, output, key);
}

void gronsfeldDecryptF(const string& input, const string& output, const string& key) {
    decryptGronsfeld(input, output, key);
}

vector<unsigned char> gronsfeldEncryptC(const string& text, const string& key) {
    return encryptGronsfeldConsole(text, key);
}

vector<unsigned char> gronsfeldDecryptC(const string& text, const string& key) {
    return decryptGronsfeldConsole(text, key);
}

extern "C" {
    CipherPlugin* createCipher() {
        CipherPlugin* cipher = new CipherPlugin;

        cipher->name = "Гронсфельд";
        
        cipher->validateKey = gronsfeldValidate;
        cipher->encryptFile = gronsfeldEncryptF;
        cipher->decryptFile = gronsfeldDecryptF;
        cipher->encryptConsole = gronsfeldEncryptC;
        cipher->decryptConsole = gronsfeldDecryptC;

        return cipher;
    }

    void destroyPlugin(CipherPlugin* p) {
        delete p;
    }
}