mkdir -p build

cd build
cmake --configure ../

cmake --build .
make
sudo make install