set(PYTHON_VERSION ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
set(PYTHON_INTERPRETER_ID ${Python3_INTERPRETER_ID})

if(ENABLE_GITEE OR ENABLE_GITEE_EULER) # Channel GITEE_EULER is NOT supported now, use GITEE instead.
    # MIPT: Use newer version of module for PyPy builds
    #       due to identation bug that was fixed only in v2.9.2+ (and many other global bugs)
    #       @see https://github.com/pybind/pybind11/commit/5f9b090a915fbaa5904c6bd4a24f888f2ff55db7#diff-2881c4e793b918439a3bba381ab0dbcb932d243676a99b899182ac34ad05363c
    if(PYTHON_INTERPRETER_ID MATCHES "PyPy")
        message("Found PyPy (Py ${PYTHON_VERSION})")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.9.2.tar.gz")
        set(SHA256 "6bd528c4dbe2276635dc787b6b1f2e5316cf6b49ee3e150264e455a0d68d19c1")
    elseif(PYTHON_VERSION MATCHES "3.7")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.4.3.tar.gz")
        set(SHA256 "182cf9e2c5a7ae6f03f84cf17e826d7aa2b02aa2f3705db684dfe686c0278b36")
    elseif(PYTHON_VERSION MATCHES "3.8")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.6.1.tar.gz")
        set(SHA256 "c840509be94ac97216c3b4a3ed9f3fdba9948dbe38c16fcfaee3acc6dc93ed0e")
    elseif(PYTHON_VERSION MATCHES "3.9")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.6.1.tar.gz")
        set(SHA256 "c840509be94ac97216c3b4a3ed9f3fdba9948dbe38c16fcfaee3acc6dc93ed0e")
    elseif(PYTHON_VERSION MATCHES "3.10")
        set(REQ_URL "https://gitee.com/mirrors/pybind11/repository/archive/v2.6.1.tar.gz")
        set(SHA256 "c840509be94ac97216c3b4a3ed9f3fdba9948dbe38c16fcfaee3acc6dc93ed0e")
    else()
        message("Could not find 'Python 3.7' or 'Python 3.8' or 'Python 3.9' or 'Python 3.10'")
        return()
    endif()
else()
    # MIPT: @see above, same here for non-gitee builds
    if(PYTHON_INTERPRETER_ID MATCHES "PyPy")
        message("Found PyPy (Py ${PYTHON_VERSION})")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.9.2.tar.gz")
        set(SHA256 "6bd528c4dbe2276635dc787b6b1f2e5316cf6b49ee3e150264e455a0d68d19c1")
    elseif(PYTHON_VERSION MATCHES "3.7")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.4.3.tar.gz")
        set(SHA256 "1eed57bc6863190e35637290f97a20c81cfe4d9090ac0a24f3bbf08f265eb71d")
    elseif(PYTHON_VERSION MATCHES "3.8")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz")
        set(SHA256 "cdbe326d357f18b83d10322ba202d69f11b2f49e2d87ade0dc2be0c5c34f8e2a")
    elseif(PYTHON_VERSION MATCHES "3.9")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz")
        set(SHA256 "cdbe326d357f18b83d10322ba202d69f11b2f49e2d87ade0dc2be0c5c34f8e2a")
    elseif(PYTHON_VERSION MATCHES "3.10")
        set(REQ_URL "https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz")
        set(SHA256 "cdbe326d357f18b83d10322ba202d69f11b2f49e2d87ade0dc2be0c5c34f8e2a")
    else()
        message("Could not find 'Python 3.7' or 'Python 3.8' or 'Python 3.9' or 'Python 3.10'")
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
            SHA256 ${SHA256}
            CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
            PATCHES ${TOP_DIR}/third_party/patch/pybind11/pybind11-pypy-embedding.patch
            PATCHES ${TOP_DIR}/third_party/patch/pybind11/pybind11-pypy-constmethods-from-262.patch
            )
elseif(PYTHON_VERSION MATCHES "3.7")
    mindspore_add_pkg(pybind11
            VER 2.4.3
            URL ${REQ_URL}
            SHA256 ${SHA256}
            CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
            )
elseif(PYTHON_VERSION MATCHES "3.8")
    mindspore_add_pkg(pybind11
            VER 2.6.1
            URL ${REQ_URL}
            SHA256 ${SHA256}
            CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
            )
else()
    mindspore_add_pkg(pybind11
            VER 2.6.1
            URL ${REQ_URL}
            SHA256 ${SHA256}
            PATCHES ${pybind11_patch}
            CMAKE_OPTION -DPYBIND11_TEST=OFF -DPYBIND11_LTO_CXX_FLAGS=FALSE
            )
endif()

include_directories(${pybind11_INC})
find_package(pybind11 REQUIRED)
set_property(TARGET pybind11::module PROPERTY IMPORTED_GLOBAL TRUE)
add_library(mindspore::pybind11_module ALIAS pybind11::module)
