#ifndef SHADLP_UTILS_HPP
#define SHADLP_UTILS_HPP

#include <string>

namespace utils
{

std::string DecimalToHexadecimal(int dec);
std::string EncodeURL(std::string data);

int HexadecimalToDecimal(std::string hex);
std::string DecodeURL(std::string data);

} // utils

#endif // SHADLP_UTILS_HPP
