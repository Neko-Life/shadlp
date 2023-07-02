
# ShaDLP

Musicat backend music storage server. My fun project to learn more about the C++ language for application backend. Nothing to read yet.

## Dependencies

* [uWebSockets](https://github.com/uNetworking/uWebSockets)
* [nlohmann/json](https://github.com/nlohmann/json/tree/develop/single_include/nlohmann) - Headers only, included

## Compiling

* Clone uWebSockets repo into `libs/`, compile and install it

```sh
mkdir libs
cd libs
git clone 'https://github.com/uNetworking/uWebSockets' --recurse-submodules
cd uWebSockets/uSockets
git checkout master
cd ..
WITH_OPENSSL=1 WITH_ZLIB=1 WITH_PROXY=1 WITH_ASAN=1 make -j$(nproc)
sudo make prefix=/usr install
cd ../..
```

* Compile

```sh
mkdir build
cd build
cmake ..
make all
```
