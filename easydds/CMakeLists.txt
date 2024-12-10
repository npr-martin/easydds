cmake_minimum_required(VERSION 3.20)

project("EasyDDS")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find requirements
find_package(fastcdr 2 REQUIRED)
find_package(fastdds 3 REQUIRED)

message(STATUS "Configuring easydds...")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../install/include)
file(GLOB INC_PATH ${PROJECT_SOURCE_DIR}/include/*.hpp)
list(APPEND INCS ${INC_PATH})
file(GLOB EASY_DDS_SOURCES_CXX ${PROJECT_SOURCE_DIR}/source/*.cxx)

add_library(${PROJECT_NAME} SHARED ${INCS} ${EASY_DDS_SOURCES_CXX})
# target_link_libraries(${project_name} fastcdr fastdds)

# compile application
# add_executable(EasyDDSApp easyddsmain.cxx ${EASY_DDS_SOURCES_CXX})
# target_link_libraries(EasyDDSApp fastcdr fastdds)



