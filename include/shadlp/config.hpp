#ifndef SHADLP_CONFIG_HPP
#define SHADLP_CONFIG_HPP

#include "nlohmann/json.hpp"

namespace config
{

int load_config(const char *file);
int set_port(int p);
int get_port();

} // config

#endif // SHADLP_CONFIG_HPP

// vim: ts=8 sw=8 noet
