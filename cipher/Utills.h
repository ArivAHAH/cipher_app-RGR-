#include "CipherInterface.h"
#include <vector>

using namespace std;

struct LoadedCipherInfo {
    CipherPlugin* cipher;
    void* handle; 
    void (*destroyFunc)(CipherPlugin*);
};


extern vector<LoadedCipherInfo> gLoadedCiphers;

void loadCiphers(const string& dirPath);
void unloadCiphers();
bool isValidNumber(const string& str, int& result);
wchar_t customToWUpper(wchar_t c);
wchar_t customToWLower(wchar_t c);
bool isValidUtf8(const vector<unsigned char>& data);

