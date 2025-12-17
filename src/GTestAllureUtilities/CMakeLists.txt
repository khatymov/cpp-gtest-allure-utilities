cmake_minimum_required(VERSION 3.15)

find_package(GTest)

add_compile_options(-D RAPIDJSON_HAS_STDSTRING=1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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


include(GNUInstallDirs)

install(TARGETS GTestAllureUtilities
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/AllureAPI.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Allure2Listener.h
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

install(DIRECTORY src/GTestAllureUtilities/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GTestAllureUtilities
  FILES_MATCHING PATTERN "*.h"
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

include(CPack)
