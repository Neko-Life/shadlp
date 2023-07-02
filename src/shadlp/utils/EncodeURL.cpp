/*
    Credits to www.programmingalgorithms.com

    Taken from https://www.programmingalgorithms.com/algorithm/url-encoding/cpp/
*/

#include <string>

namespace utils
{

using string = std::string;

string DecimalToHexadecimal(int dec)
{
	if (dec < 1)
		return "0";

	int hex = dec;
	string hexStr;

	while (dec > 0) {
		hex = dec % 16;

		if (hex < 10)
			hexStr = hexStr.insert(0, string(1, (char)(hex + 48)));
		else
			hexStr = hexStr.insert(0, string(1, (char)(hex + 55)));

		dec /= 16;
	}

	return hexStr;
}

string EncodeURL(string data)
{
	string result;

	for (char c : data) {
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
		    ('0' <= c && c <= '9')) {
			result += c;
		} else {
			result += '%';

			string s = DecimalToHexadecimal(c);

			if (s.length() < 2)
				s = s.insert(0, "0");

			result += s;
		}
	}

	return result;
}

} // utils
