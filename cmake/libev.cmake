set(LIBEV_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libev")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libev")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libev/include")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libev/lib")

file(
  WRITE "${LIBEV_PREFIX}/Makefile.am.patch"
  "
--- Makefile.am	2019-10-31 04:02:23.000000000 +0000
+++ Makefile_new.am	2022-07-18 15:40:44.000000000 +0100
@@ -10,7 +10,7 @@

 man_MANS = ev.3

-include_HEADERS = ev.h ev++.h event.h
+include_HEADERS = ev.h

 lib_LTLIBRARIES = libev.la
")

include(ExternalProject)

ExternalProject_Add(
  libev
  URL "http://dist.schmorp.de/libev/Attic/libev-4.33.tar.gz"
  URL_HASH
    SHA256=507eb7b8d1015fbec5b935f34ebed15bf346bed04a11ab82b8eee848c4205aea
  PATCH_COMMAND patch < ${LIBEV_PREFIX}/Makefile.am.patch && aclocal && automake
  PREFIX ${LIBEV_PREFIX}
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --disable-dependency-tracking
                    --enable-shared=no --prefix=${LIBEV_PREFIX}
  BUILD_COMMAND make
  INSTALL_COMMAND
    make install && mkdir ${LIBEV_PREFIX}/include/ev && mv
    ${LIBEV_PREFIX}/include/ev.h ${LIBEV_PREFIX}/include/ev/ev.h && rm
    ${LIBEV_PREFIX}/lib/libev.la)

add_library(ev STATIC IMPORTED GLOBAL)
add_dependencies(ev libev)
set_target_properties(ev PROPERTIES IMPORTED_LOCATION
                                    ${LIBEV_PREFIX}/lib/libev.a)
set_target_properties(ev PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                    ${LIBEV_PREFIX}/include)
