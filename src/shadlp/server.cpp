#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <condition_variable>
#include <uWebSockets/App.h>
#include "shadlp/server.hpp"
#include "shadlp/config.hpp"
#include "shadlp/utils.hpp"

#define UWS_WITH_SSL false
#define DASHBOARD_URI "dashboard/"
#define DASHBOARD_APP_PATH "app/"
#define DASHBOARD_URI_LENGTH sizeof(DASHBOARD_URI) / sizeof(DASHBOARD_URI[0])
#define DASHBOARD_APP_PATH_LENGTH \
	sizeof(DASHBOARD_APP_PATH) / sizeof(DASHBOARD_APP_PATH[0])

namespace server
{

static int running = 0;

// assign null to these pointer on exit!
static uWS::App *_app_ptr = nullptr;
static uWS::Loop *_loop = nullptr;

enum filepath_t {
	REG_FILE = 0,
	DIRECTORY,
};

struct response_data_t {
	std::vector<char> data;
	size_t size;
};

std::map<std::string, response_data_t> app_files_cache;

std::string get_filename_extension(const std::string &filename)
{
	size_t idx = filename.find_last_of('.');

	if (idx != std::string::npos && idx < (filename.length() - 1))
		return filename.substr(idx + 1);

	return "";
}

int fs_is_regular(struct stat *filestat)
{
	return S_ISREG(filestat->st_mode);
}

int fs_is_directory(struct stat *filestat)
{
	return S_ISDIR(filestat->st_mode);
}

int invalid_regular_fd(struct stat *filestat, FILE *file)
{
	if (file) {
		fstat(fileno(file), filestat);
		return !fs_is_regular(filestat);
	} else
		return 1;
}

int file_exist(const char *path, filepath_t is_type = REG_FILE)
{
	struct stat filestat;
	stat(path, &filestat);

	switch (is_type) {
	case REG_FILE:
		return fs_is_regular(&filestat);
	case DIRECTORY:
		return fs_is_directory(&filestat);
	default:
		return 0;
	}
}

int load_app_file_to_cache(const char *path)
{
	int err = 0;

	struct stat filestat;
	FILE *file = fopen(path, "r");
	fstat(fileno(file), &filestat);

	size_t buffer_size = filestat.st_size;
	std::vector<char> buffer(buffer_size);

	size_t has_read = 0;
	size_t current_read = 0;

	while ((current_read = fread(&buffer[has_read], 1,
				     buffer_size - has_read, file)) > 0) {
		has_read += current_read;
	}

	const std::string key =
		std::string("/") + DASHBOARD_URI +
		std::string(path).substr(DASHBOARD_APP_PATH_LENGTH - 1);
	app_files_cache.insert({ key, { buffer, buffer_size } });

	if (file)
		fclose(file);

	fprintf(stderr, "%s\n", key.c_str());
	return err;
}

int load_app_files_cache(const char *path)
{
	DIR *app_dir = opendir(path);
	if (!app_dir) {
		fprintf(stderr,
			"[server ERROR] Can't open dashboard app directory\n");
		return 1;
	}

	int err = 0;

	struct dirent *app_dirent = NULL;
	errno = 0;

	while ((app_dirent = readdir(app_dir)) != NULL) {
		errno = 0;

		if (app_dirent->d_type == DT_DIR) {
			if (!strncmp(".", app_dirent->d_name, 1) ||
			    !strncmp("..", app_dirent->d_name, 2))
				continue;

			if ((err = load_app_files_cache((std::string(path) +
							 app_dirent->d_name +
							 "/")
								.c_str())) != 0)
				break;
			continue;
		}

		else if (app_dirent->d_type == DT_REG) {
			if ((err = load_app_file_to_cache(
				     (std::string(path) + app_dirent->d_name)
					     .c_str())) != 0)
				break;
		}
	}

	if (errno) {
		fprintf(stderr, "[server ERROR] Error reading '%s': %d\n", path,
			errno);
		errno = 0;
		err = 2;
	}

	if (app_dir)
		closedir(app_dir);

	return err;
}

int respond_stream_file(uWS::HttpResponse<UWS_WITH_SSL> *res,
			const char *filename)
{
	static const size_t BUFFER_SIZE = 2048 * 1000;

	int err = 0;
	std::string res_status = "200", res_body = "OK";

	struct stat filestat;
	FILE *file = fopen(filename, "r");

	if (invalid_regular_fd(&filestat, file)) {
		res_status = "404";
		res_body = "Not Found";

		res->writeStatus(res_status);
		res->end(res_body, true);

		if (file)
			fclose(file);

		return err;
	}

	size_t current_read = 0;
	size_t fail_write_count = 0;
	char buffer[BUFFER_SIZE];

	while ((current_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
		if (!res->write({ buffer, current_read })) {
			// failed sending data
			fprintf(stderr,
				"[server WARN] Failed streaming '%s'\n"
				"at byte: %ld\n"
				"write offset: %ld\n"
				"current read: %ld\n"
				"count: %ld\n",
				filename, ftell(file), res->getWriteOffset(),
				current_read, ++fail_write_count);

			// probably wanna keep track of failed write attempt
			// and abort at certain threshold
			/*
			err = 2;
			break;
			*/
		}
	}

	if (err == 0)
		res->end("", true);
	else
		// empty abort handler cuz what else to do?
		res->onAborted([]() {});

	if (file)
		fclose(file);

	return err;
}

std::function<void(uWS::HttpResponse<UWS_WITH_SSL> *, uWS::HttpRequest *)>
get_static_serve_handler(const std::string path,
			 const bool is_dashboard = false)
{
	// !TODO: this is broken, help pls...
	// i think browsers requires Content-Type header to be correctly
	// populated to load files correctly, but binary file streaming
	// works without it???
	if (is_dashboard) {
		return [path](uWS::HttpResponse<UWS_WITH_SSL> *res,
			      uWS::HttpRequest *req) {
			std::cout << "req: " << req->getUrl() << '\n';

			auto cache = app_files_cache.find(
				std::string(req->getUrl()));

			if (cache == app_files_cache.end()) {
				std::string new_key = std::string("/") +
						      DASHBOARD_URI +
						      "index.html";
				std::cout << req->getUrl()
					  << " key not found, fallback: "
					  << new_key << '\n';

				cache = app_files_cache.find(new_key);
			}

			if (cache == app_files_cache.end()) {
				std::cout << "new key not found " << '\n';
				res->writeStatus("404");
				res->end("Not Found", true);
			} else
				res->end({ &cache->second.data[0],
					   cache->second.size });
		};
	}

	return [path](uWS::HttpResponse<UWS_WITH_SSL> *res,
		      uWS::HttpRequest *req) {
		int err = respond_stream_file(
			res, (path + utils::DecodeURL(
					     std::string(req->getParameter(0))))
				     .c_str());
	};
}

int run_server()
{
	if (running) {
		fprintf(stderr, "[server ERROR] Instance already running!\n");
		return 1;
	}

	const int PORT = config::get_port();
	if (!PORT) {
		fprintf(stderr, "[server] No port provided, exiting...");
		return 1;
	}

	uWS::App app;

	// initialize global pointers, assign null to these pointer on exit!
	_app_ptr = &app;
	_loop = uWS::Loop::get();

	// define api routes ======================================={

	app.get("/check/:id", [](uWS::HttpResponse<UWS_WITH_SSL> *res,
				 uWS::HttpRequest *req) {
		const std::string id(req->getParameter(0));

		nlohmann::json response;

		response["stored"] = file_exist(("music/" + id).c_str());
		response["meta"] = file_exist(("meta/" + id).c_str());

		res->end(response.dump(), true);
	});

	app.get("/music/:id", get_static_serve_handler("music/"));
	app.get("/meta/:id", get_static_serve_handler("meta/"));

	int err = load_app_files_cache(DASHBOARD_APP_PATH);

	if (err == 0)
		app.get("/" DASHBOARD_URI "*",
			get_static_serve_handler(DASHBOARD_APP_PATH, true));
	else
		fprintf(stderr,
			"[server WARN] Can't load app dashboard, dashboard is disabled\n");

	// define api routes =======================================}

	app.listen(PORT, [PORT](auto *listen_socket) {
		if (listen_socket) {
			fprintf(stderr, "[server] Listening on port %d\n",
				PORT);
		}
	});

	running = 1;
	app.run(); // shouldn't return at all
	running = 0;

	// socket exiting, might be error or signal, assigning null to these pointer
	_app_ptr = nullptr;
	_loop = nullptr;

	fprintf(stderr, "[server] Exiting...");

	return 0;
}

} // server

// vim: ts=8 sw=8 noet
