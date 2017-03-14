cmake_minimum_required (VERSION 2.6)
project (eventtutorial CXX)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS} -DHR_PROTO_EXPORT=")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 11)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHR_PROTO_EXPORT=")
endif ()
  
set (HOTROD_PACKAGE_DIR /home/rigazilla/git/cpp-client/build/_CPack_Packages/RHEL-x86_64/RPM/infinispan-hotrod-cpp-8.1.0-SNAPSHOT-RHEL-x86_64)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${HOTROD_PACKAGE_DIR}/usr/include)

link_directories(${HOTROD_PACKAGE_DIR}/usr/lib)

add_executable(quickstart src/quickstart.cpp)
target_link_libraries(quickstart hotrod hotrod_protobuf)
