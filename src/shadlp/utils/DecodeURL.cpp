/*
    Credits to www.programmingalgorithms.com

    Taken from https://www.programmingalgorithms.com/algorithm/url-decoding/cpp/
*/

#include <string>
#include <cmath>

namespace utils
{

using string = std::string;

int HexadecimalToDecimal(string hex)
{
	int hexLength = hex.length();
	double dec = 0;

	for (int i = 0; i < hexLength; ++i) {
		char b = hex[i];

		if (b >= 48 && b <= 57)
			b -= 48;
		else if (b >= 65 && b <= 70)
			b -= 55;

		dec += b * pow(16, ((hexLength - i) - 1));
	}

	return (int)dec;
}

string DecodeURL(string data)
{
	string result;
	int dataLen = data.length();

	for (int i = 0; i < dataLen; ++i) {
		if (data[i] == '%') {
			result += (char)HexadecimalToDecimal(
				data.substr(i + 1, 2));
			i += 2;
		} else {
			result += data[i];
		}
	}

	return result;
}

} // utils
