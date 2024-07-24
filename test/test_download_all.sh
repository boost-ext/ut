#!/bin/sh
# run this from project directory
rm -rf ./{build,build2,install_dir}
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_QUIET=OFF -DCPM_DOWNLOAD_ALL=1 --log-level=DEBUG -DBOOST_UT_ENABLE_INSTALL=1
cmake --build ./build/ -j24 --verbose
cmake --install ./build/ --prefix ./install_dir
CMAKE_PREFIX_PATH=./install_dir/ cmake -Stest -Bbuild2 -DTEST_INSTALLED_VERSION=1 -DCPM_DOWNLOAD_ALL=1 --log-level=DEBUG

