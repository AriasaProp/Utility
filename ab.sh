set -e
cmake -S . -B build
cmake --build build -t Utility_test --config Release --clean-first
./build/Utility_test