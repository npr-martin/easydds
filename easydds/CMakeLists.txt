
cmake_minimum_required(VERSION 3.20)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastdds 3 REQUIRED)


message(STATUS "Configuring easydds...")
add_library(easydds_lib easyddsTypeObjectSupport.cxx easyddsPubSubTypes.cxx)
target_link_libraries(easydds_lib fastcdr fastdds)

add_executable(easydds easyddsApplication.cxx easyddsPublisherApp.cxx easyddsSubscriberApp.cxx easyddsmain.cxx)
target_link_libraries(easydds fastcdr fastdds
        easydds_lib 
        )



