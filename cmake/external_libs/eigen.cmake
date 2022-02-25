set(Eigen3_CXXFLAGS "-D_FORTIFY_SOURCE=2 -O2")
set(Eigen3_CFLAGS "-D_FORTIFY_SOURCE=2 -O2")

set(REQ_URL "https://gitlab.com/libeigen/eigen/-/archive/3.3.9/eigen-3.3.9.tar.gz")
set(MD5 "609286804b0f79be622ccf7f9ff2b660")

mindspore_add_pkg(
  Eigen3
  VER 3.3.9
  URL ${REQ_URL}
  MD5 ${MD5}
  CMAKE_OPTION -DBUILD_TESTING=OFF
  TARGET_ALIAS mindspore::eigen Eigen3::Eigen)
