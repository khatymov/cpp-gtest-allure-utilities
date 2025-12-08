cmake_minimum_required(VERSION 3.15)

# Find external dependencides
find_package(RapidJSON REQUIRED)
# find_package(JSONAdapterInterface)

# Configure preprocessor definitions
add_compile_options(-DRAPIDJSON_HAS_STDSTRING=1)
add_compile_options(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

# Add project folder into includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Configure RapidJSONAdapter static library
set(RAPIDJSON_ADAPTER RapidJSONAdapter)
file(GLOB_RECURSE RAPIDJSON_ADAPTER_SRC "*.cpp")
file(GLOB_RECURSE RAPIDJSON_ADAPTER_HDR "*.h")
add_library(${RAPIDJSON_ADAPTER} STATIC ${RAPIDJSON_ADAPTER_SRC} ${RAPIDJSON_ADAPTER_HDR})
target_link_libraries(${RAPIDJSON_ADAPTER} rapidjson JSONAdapterInterface)

#Configure source groups
foreach(FILE ${RAPIDJSON_ADAPTER_SRC} ${RAPIDJSON_ADAPTER_HDR}) 
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
       set(GROUP "Header Files${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()
