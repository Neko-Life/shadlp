#include <sys/stat.h>
#include <fstream>
#include <condition_variable>
#include <uWebSockets/App.h>
#include "shadlp/server.hpp"
#include "shadlp/config.hpp"
#include "shadlp/utils.hpp"

namespace server
{

static bool running = false;

// assign null to these pointer on exit!
static uWS::App *_app_ptr = nullptr;
static uWS::Loop *_loop = nullptr;

int respond_stream_file(uWS::HttpResponse<false> *res, const char *filename)
{
	static const size_t BUFFER_SIZE = 2048 * 1000;

	int err = 0;
	std::string res_status = "200", res_body = "OK";

	struct stat filestat;
	FILE *file = fopen(filename, "r");

	if (file) {
		fstat(fileno(file), &filestat);
		err = !S_ISREG(filestat.st_mode);
	} else
		err = 1;

	if (err) {
		res_status = "404";
		res_body = "Not Found";

		res->writeStatus(res_status);
		res->end(res_body, true);

		if (file)
			fclose(file);

		return err;
	}

	size_t current_read = 0;
	char buffer[BUFFER_SIZE];

	while ((current_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
		res->write({ buffer, current_read });
	}

	res->end();

	if (file)
		fclose(file);

	return err;
}

std::function<void(uWS::HttpResponse<false> *, uWS::HttpRequest *)>
get_static_serve_handler(std::string path)
{
	return [path](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
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

	app.get("/music/:filename", get_static_serve_handler("music/"));
	app.get("/meta/:filename", get_static_serve_handler("meta/"));

	// define api routes =======================================}

	app.listen(PORT, [PORT](auto *listen_socket) {
		if (listen_socket) {
			fprintf(stderr, "[server] Listening on port %d\n",
				PORT);
		}
	});

	running = true;
	app.run();

	// socket exiting, assigning null to these pointer
	_app_ptr = nullptr;
	_loop = nullptr;

	fprintf(stderr, "[server] Exiting...");

	return 0;
}

} // server

// vim: ts=8 sw=8 noet
