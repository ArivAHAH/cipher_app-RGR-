#include <string>
#include <vector>

using namespace std;

void validateKey(const string& cipherType, const string& key);
void encryptGronsfeld(const string& inputFilePath, const string& outputFilePath, const string& key);
void decryptGronsfeld(const string& inputFilePath, const string& outputFilePath, const string& key);
void encryptDoubleTransposition(const string& inputFilePath, const string& outputFilePath, const string& key);
void decryptDoubleTransposition(const string& inputFilePath, const string& outputFilePath, const string& key);
void encryptVigenere(const string& inputFilePath, const string& outputFilePath, const string& key);
void decryptVigenere(const string& inputFilePath, const string& outputFilePath, const string& key);
vector<unsigned char> encryptGronsfeldConsole(const string& inputText, const string& key);
vector<unsigned char> decryptGronsfeldConsole(const string& inputText, const string& key);
vector<unsigned char> encryptDoubleTranspositionConsole(const string& inputText, const string& key);
vector<unsigned char> decryptDoubleTranspositionConsole(const string& inputText, const string& key);
vector<unsigned char> encryptVigenereConsole(const string& inputText, const string& key);
vector<unsigned char> decryptVigenereConsole(const string& inputText, const string& key);