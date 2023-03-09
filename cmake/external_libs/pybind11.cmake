set(PYTHON_VERSION ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
set(PYTHON_INTERPRETER_ID ${Python3_INTERPRETER_ID})

if(ENABLE_GITEE)
    # MIPT: Use newer version of module for PyPy builds
    #       due to identation bug that was fixed only in v2.9.2+
    #       @see https://github.com/pybind/pybind11/commit/5f9b090a915fbaa5904c6bd4a24f888f2ff55db7#diff-2881c4e793b918439a3bba381ab0dbcb932d243676a99b899182ac34ad05363c
    if(PYTHON_INTERPRETER_ID MATCHES "PyPy")
        message("Found PyPy (Py ${PYTHON_VERSION})")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.9.2.tar.gz")
        set(MD5 "06e4b6c2d0a5d6c6025941203cfcd4b6")
    elseif(PYTHON_VERSION MATCHES "3.9")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.6.1.tar.gz")
        set(MD5 "cd04f7bd275fedb97e8b583c115769e6")
    elseif(PYTHON_VERSION MATCHES "3.8")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.6.1.tar.gz")
        set(MD5 "cd04f7bd275fedb97e8b583c115769e6")
    elseif(PYTHON_VERSION MATCHES "3.7")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.4.3.tar.gz")
        set(MD5 "8f69438201bc824c63e5774bf8c1d422")
    else()
        message("Could not find 'Python 3.8' or 'Python 3.7' or 'Python 3.9'")
        return()
    endif()
else()
    # MIPT: @see above, same here for non-gitee builds
    if(PYTHON_INTERPRETER_ID MATCHES "PyPy")
        message("Found PyPy (Py ${PYTHON_VERSION})")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.9.2.tar.gz")
        set(MD5 "06e4b6c2d0a5d6c6025941203cfcd4b6")
    elseif(PYTHON_VERSION MATCHES "3.9")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz")
        set(MD5 "32a7811f3db423df4ebfc731a28e5901")
    elseif(PYTHON_VERSION MATCHES "3.8")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz")
        set(MD5 "32a7811f3db423df4ebfc731a28e5901")
    elseif(PYTHON_VERSION MATCHES "3.7")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.4.3.tar.gz")
        set(MD5 "62254c40f89925bb894be421fe4cdef2")
    else()
        message("Could not find 'Python 3.8' or 'Python 3.7' or 'Python 3.9'")
        return()
    endif()
endif()
set(pybind11_CXXFLAGS "-D_FORTIFY_SOURCE=2 -O2")
set(pybind11_CFLAGS "-D_FORTIFY_SOURCE=2 -O2")
set(pybind11_patch ${TOP_DIR}/third_party/patch/pybind11/pybind11.patch001)

# MIPT: Use newer version of module for PyPy builds with additional patches.
# NOTE: 1st patch suppresses error message about including embed.h in PyPy builds
#       (mindspore seems not to use embedding mode anyway),
#       2nd patch rolls back 'const' qualifier for for v2.9.2's
#       pybind11::list::append() method to align with mindspore code that was designed for v2.6.2.
# NOTE: existing patch (pybind11.patch001) is not needed anymore for v2.9.2.
if(PYTHON_INTERPRETER_ID MATCHES "PyPy")
    mindspore_add_pkg(pybind11
        VER 2.9.2
        URL ${REQ_URL}
        MD5 ${MD5}
        CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
        PATCHES ${TOP_DIR}/third_party/patch/pybind11/pybind11-pypy-embedding.patch
        PATCHES ${TOP_DIR}/third_party/patch/pybind11/pybind11-pypy-constmethods-from-262.patch
        )
elseif(PYTHON_VERSION MATCHES "3.9")
    mindspore_add_pkg(pybind11
        VER 2.6.1
        URL ${REQ_URL}
        MD5 ${MD5}
        PATCHES ${pybind11_patch}
        CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
        )
elseif(PYTHON_VERSION MATCHES "3.8")
    mindspore_add_pkg(pybind11
        VER 2.6.1
        URL ${REQ_URL}
        MD5 ${MD5}
        CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
        )
else()
    mindspore_add_pkg(pybind11
        VER 2.4.3
        URL ${REQ_URL}
        MD5 ${MD5}
        CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
        )
endif()

include_directories(${pybind11_INC})
find_package(pybind11 REQUIRED)
set_property(TARGET pybind11::module PROPERTY IMPORTED_GLOBAL TRUE)
add_library(mindspore::pybind11_module ALIAS pybind11::module)
