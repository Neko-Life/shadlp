#include <fstream>
#include "shadlp/config.hpp"
#include "nlohmann/json.hpp"

namespace config
{

static int port = 0;

int load_config(const char *file)
{
	int err = 0;
	nlohmann::json config = nullptr;

	std::ifstream scs(file);
	if (!scs.is_open())
		return 1;

	try {
		scs >> config;

		if (config.is_null()) {
			fprintf(stderr, "[ERROR] Config file is empty\n");
			err = 2;
		} else if (!config.is_object()) {
			fprintf(stderr,
				"[ERROR] Invalid config json, json should be an object\n");
			err = 3;
		}
	} catch (nlohmann::json::exception &error) {
		fprintf(stderr, "[ERROR] Parsing config:\n%s\n", error.what());
		err = error.id;
	}

	scs.close();

	if ((err = set_port(config.value("PORT", 0))))
		return err;

	return err;
}

int set_port(int p)
{
	if (p > 0 && p <= 65535)
		port = p;
	else {
		fprintf(stderr, "[ERROR] Invalid port: %d\n", p);
		return 1;
	}

	return 0;
}

int get_port()
{
	return port;
}

} // config

// vim: ts=8 sw=8 noet
