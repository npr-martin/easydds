cmake_minimum_required(VERSION 3.20)

project("EasyDDS")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_FLAGS "-pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find requirements

#set(fastcdr_DIR ${PROJECT_SOURCE_DIR}/../install/lib/cmake/fastcdr)
#find_package(fastcdr 2 REQUIRED)

#find_package(foonathan_memory-0.7.3 PATHS ${PROJECT_SOURCE_DIR}/../install/lib NO_DEFAULT_PATH)
#find_package(fastcdr PATHS ${PROJECT_SOURCE_DIR}/../install/lib NO_DEFAULT_PATH)
#find_package(fastdds PATHS ${PROJECT_SOURCE_DIR}/../install/lib NO_DEFAULT_PATH)

message(STATUS "Configuring easydds...")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../install/include)
file(GLOB INC_PATH ${PROJECT_SOURCE_DIR}/include/*.hpp)
list(APPEND INCS ${INC_PATH})
file(GLOB EASY_DDS_SOURCES_CXX ${PROJECT_SOURCE_DIR}/source/*.cxx)
file(GLOB MONITOR_SOURCES_CXX ${PROJECT_SOURCE_DIR}/source/monitorSevice.cpp)

add_library(${PROJECT_NAME} SHARED ${INCS} ${EASY_DDS_SOURCES_CXX})

install(TARGETS ${PROJECT_NAME}
     DESTINATION ${PROJECT_SOURCE_DIR}/../install/lib)

add_executable(monitor ${MONITOR_SOURCES_CXX})

#target_link_directories(monitor PRIVATE ${PROJECT_SOURCE_DIR}/build)
target_link_directories(monitor PRIVATE ${PROJECT_SOURCE_DIR}/../install/lib)
target_link_directories(monitor PRIVATE ${PROJECT_SOURCE_DIR}/../install/bin)
target_link_libraries(monitor fastcdr foonathan_memory-0.7.3 fastdds EasyDDS)
install(TARGETS monitor
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../install/bin)



