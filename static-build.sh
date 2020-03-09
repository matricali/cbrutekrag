#!/bin/bash
#
# Copyright (c) 2014-2020 Jorge Matricali
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

LIBSSH_VERSION="0.9.3"
LIBSSH_DEST="external/libssh-${LIBSSH_VERSION}"
LIBSSH_BUILD_DIR="${LIBSSH_DEST}/build"

if [ ! -d "${LIBSSH_DEST}" ]; then
    mkdir -p external
    wget --no-check-certificate 'https://www.libssh.org/files/0.9/libssh-0.9.3.tar.xz' -O external/libssh-0.9.3.tar.xz
    tar xf external/libssh-0.9.3.tar.xz -C external/
fi

mkdir -p "${LIBSSH_BUILD_DIR}"
pushd "${LIBSSH_BUILD_DIR}" || exit 1
cmake ../ -DWITH_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF -DWITH_STATIC_LIB=ON
make
popd || exit 1

make -f Makefile.static clean all
