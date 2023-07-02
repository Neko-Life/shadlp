# minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project name, version and description
project(shadlp
	VERSION 0.0.1
	LANGUAGES C CXX
	DESCRIPTION "Musicat backend music storage server"
)
 
# Add DPP as dependency
# add_subdirectory(libs/DPP)
# You can also add any other libs you want to use
 
# Create an executable
add_executable(shadlp
	# include files
	include/nlohmann/json.hpp
	include/shadlp/config.hpp
	include/shadlp/server.hpp
	include/shadlp/utils.hpp
	include/main.hpp
	include/version.hpp

	# impl files
	src/shadlp/utils/DecodeURL.cpp
	src/shadlp/utils/EncodeURL.cpp
	src/shadlp/config.cpp
	src/shadlp/server.cpp
	src/main.cpp
)

 
# Linking libraries
target_link_libraries(shadlp
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/bsd.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/context.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/epoll_kqueue.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/gcd.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/libuv.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/loop.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/openssl.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/quic.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/sni_tree.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/socket.o
	${CMAKE_CURRENT_SOURCE_DIR}/libs/uWebSockets/uSockets/udp.o
	z
	ssl
	crypto
	# Add any other libs you want to use here
)
 
# Specify includes
target_include_directories(shadlp PRIVATE
	include
	libs/uWebSockets/uSockets/src
	# Remember to add the include directories of any other libraries too
)

target_compile_options(shadlp PRIVATE -g -Wall -Wextra -Wpedantic)
 
# Set C++ version
set_target_properties(shadlp PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	EXPORT_COMPILE_COMMANDS ON
)

# vim: ts=8 sw=8 noet
