$ErrorActionPreference = "Stop"

$JOBS=8
$INSTALL_DIR="./deploy"

mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="../$INSTALL_DIR"
cd ..
cmake --build build --config Release -j $JOBS
