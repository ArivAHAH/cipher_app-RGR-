#include <string>
#include <vector>

using namespace std;

struct CipherPlugin {
    const char* name;

    void (*validateKey)(const string& key);
    void (*encryptFile)(const string& input, const string& output, const string& key);
    void (*decryptFile)(const string& input, const string& output, const string& key);
    vector<unsigned char> (*encryptConsole)(const string& text, const string& key);
    vector<unsigned char> (*decryptConsole)(const string& text, const string& key);
};

