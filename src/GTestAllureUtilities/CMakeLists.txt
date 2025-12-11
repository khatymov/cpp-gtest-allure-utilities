cmake_minimum_required(VERSION 3.15)

# Find external dependencides
find_package(GTest)
# find_package(RapidJSONAdapter)

# Configure preprocessor definitions
add_compile_options(-D RAPIDJSON_HAS_STDSTRING=1)

# Add project folder into includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Configure GTestAllureUtilities static library
set(GTEST_ALLURE_UTILITIES GTestAllureUtilities)
file(GLOB_RECURSE GTEST_ALLURE_UTILITIES_SRC "*.cpp")
file(GLOB_RECURSE GTEST_ALLURE_UTILITIES_HDR "*.h")

add_library(${GTEST_ALLURE_UTILITIES} STATIC ${GTEST_ALLURE_UTILITIES_SRC} ${GTEST_ALLURE_UTILITIES_HDR})

add_subdirectory(interfaces)
add_subdirectory(RapidJSONAdapter)

target_link_libraries(${GTEST_ALLURE_UTILITIES} GTest::GTest RapidJSONAdapter)

target_include_directories(${GTEST_ALLURE_UTILITIES}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Model>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Services>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RapidJSONAdapter>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interfaces/JSONAdapterInterface>
        $<INSTALL_INTERFACE:include/GTestAllureUtilities>
)

install(TARGETS ${GTEST_ALLURE_UTILITIES}
    EXPORT GTestAllureUtilitiesTargets
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include/GTestAllureUtilities
)

### NEW: install headers (public API)
install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/AllureAPI.h
    DESTINATION include/GTestAllureUtilities
)

install(
    DIRECTORY
        Model
        Services
        RapidJSONAdapter
        interfaces/JSONAdapterInterface
    DESTINATION include/GTestAllureUtilities
    FILES_MATCHING
        PATTERN "*.h"
)


#Configure source groups
foreach(FILE ${GTEST_ALLURE_UTILITIES_SRC} ${GTEST_ALLURE_UTILITIES_HDR}) 
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


