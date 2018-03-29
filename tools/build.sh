#!/usr/bin/env bash

# A Bash script to build and install devel ASB on Ubuntu (16.04).

opwd=$(pwd)
function my_exit()
{
    local error_code=$1
    shift 1
    cd ${opwd}
    exit ${error_code}
}

function my_usage()
{
    cat<<EOF

Options:

   --help               : Print help

EOF
    return
}


src_dir=$(pwd)
install_dir=$(pwd)/_install.d
build_dir=$(pwd)/_build.d

build_only=0
devel=false

while [ -n "$1" ]; do
    opt="$1"
    if [ "${opt}" = "--build-only" ]; then
	build_only=1
    else
	echo >&2 "[error] Invalid command line switch '${opt}'!"
	exit 1
    fi
    shift 1
done

if [ -d ${install_dir} ]; then
    rm -fr ${install_dir}
fi

if [ -d ${build_dir} ]; then
    rm -fr ${build_dir}
fi

mkdir -p ${build_dir}

which flquery > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo >&2 "[error] Cannot find flquery! Abort!"
    my_exit 1
fi
falaise_prefix=$(flquery --prefix)
falaise_version=$(flquery --version)
echo >&2 "[info] Falaise prefix  : ${falaise_prefix}"
echo >&2 "[info] Falaise version : ${falaise_version}"

cd ${build_dir}
echo >&2 ""
echo >&2 "[info] Configuring..."
falaise_option=
if [ -n "${falaise_option}" ]; then
    falaise_option="-DFALAISE_DIR:PATH=$(flquery --cmakedir)"
fi

cmake \
    -DCMAKE_INSTALL_PREFIX="${install_dir}" \
    ${falaise_option} \
    -GNinja \
    ${src_dir}

if [ $? -ne 0 ]; then
    echo >&2 "[error] CMake failed! Abort!"
    my_exit 1
fi

echo >&2 ""
echo >&2 "[info] Building..."
ninja -j4
if [ $? -ne 0 ]; then
    echo >&2 "[error] Build failed! Abort!"
    my_exit 1
fi

if [ ${build_only} -eq 0 ]; then
    echo >&2 ""
    echo >&2 "[info] Testing..."
    ninja test
    if [ $? -ne 0 ]; then
	echo >&2 "[error] Some tests failed! Abort!"
	my_exit 1
    fi

    echo >&2 ""
    echo >&2 "[info] Installing..."
    ninja install
    if [ $? -ne 0 ]; then
	echo >&2 "[error] Installation failed! Abort!"
	my_exit 1
    fi
    tree ${install_dir}
fi


my_exit 0

# end
