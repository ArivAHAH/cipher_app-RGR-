#include <string>
#include <vector>
#include <cstdint>

using namespace std;

vector<unsigned char> processGronsfeld(const vector<unsigned char>& data, const string& key, bool encrypt, bool isText);
vector<unsigned char> processDoubleTransposition(const vector<unsigned char>& data, const string& key, bool encrypt, bool isText, uint64_t originalSize = 0);
vector<unsigned char> processVigenere(const vector<unsigned char>& data, const string& key, bool encrypt, bool isText);