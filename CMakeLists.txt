project(rabbitreader)
cmake_minimum_required(VERSION 2.8) # Проверка версии CMake.
#set(CMAKE_VERBOSE_MAKEFILE ON)
file(GLOB rr_SRC
    "*.h"
    "*.c"
    "*.hpp"
    "*.cpp"
)
find_package(Boost COMPONENTS thread program_options filesystem system regex REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
find_library(AMQP_LIBRARY NAMES rabbitmq)
find_library(Daemon_LIBRARY NAMES daemon)
find_library(MONGO_LIBRARY NAMES mongoclient)
include_directories( ${Boost_INCLUDE_DIRS} )

## Compiler flags
#if(CMAKE_COMPILER_IS_GNUCXX)
#    set(CMAKE_CXX_FLAGS "-O2")        ## Optimize
#    set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
#else()
set(CMAKE_BUILD_TYPE Debug)
#endif()


set(TLIBS_LIST ${AMQP_LIBRARY} ${MONGO_LIBRARY} ${Boost_LIBRARIES} ${OPENSSL_LIBS} ${Daemon_LIBRARY})
#message("3:" ${TLIBS_LIST})
add_executable(rabbitreader ${rr_SRC})

target_link_libraries(rabbitreader ${TLIBS_LIST} ${CMAKE_THREAD_LIBS_INIT} ssl crypto)
#set(CPACK_DEBIAN_PACKAGE_VERSION "1.0.0-1")
#set(CPACK_GENERATOR DEBIAN)
INSTALL_TARGETS(/local/bin rabbitreader)
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "openssl,libdaemon0,libboost-filesystem1.49.0,libboost-program-options1.49.0,libboost-system1.49.0,libboost-regex1.49.0,libboost-thread1.49.0")
set(CPACK_PACKAGE_DESCRIPTION "Utility/daemon for dumping JSON messages from rabbitmq into mongodb")
set(CPACK_PACKAGE_CONTACT "Maxim Sinev m.sinev@sportradar.com")
#set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")

include(CPack)
#end