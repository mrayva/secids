cmake_minimum_required(VERSION 3.20)

project(
    secids
    VERSION 0.1.0
    DESCRIPTION "Header-only C++20 ISIN to uint64_t encoder/decoder"
    HOMEPAGE_URL "https://github.com/mrayva/secids"
    LANGUAGES CXX
)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(secids_isin64 INTERFACE)
add_library(secids::isin64 ALIAS secids_isin64)
set_target_properties(secids_isin64 PROPERTIES EXPORT_NAME isin64)

target_include_directories(secids_isin64
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(secids_isin64 INTERFACE cxx_std_20)

add_executable(secids_isin64_cli src/isin64_cli.cpp)
target_link_libraries(secids_isin64_cli PRIVATE secids::isin64)
target_compile_features(secids_isin64_cli PRIVATE cxx_std_20)

add_executable(secids_cusip64_cli src/cusip64_cli.cpp)
target_link_libraries(secids_cusip64_cli PRIVATE secids::isin64)
target_compile_features(secids_cusip64_cli PRIVATE cxx_std_20)

add_executable(secids_sedol64_cli src/sedol64_cli.cpp)
target_link_libraries(secids_sedol64_cli PRIVATE secids::isin64)
target_compile_features(secids_sedol64_cli PRIVATE cxx_std_20)

add_executable(secids_figi64_cli src/figi64_cli.cpp)
target_link_libraries(secids_figi64_cli PRIVATE secids::isin64)
target_compile_features(secids_figi64_cli PRIVATE cxx_std_20)

add_executable(secids_ric64_cli src/ric64_cli.cpp)
target_link_libraries(secids_ric64_cli PRIVATE secids::isin64)
target_compile_features(secids_ric64_cli PRIVATE cxx_std_20)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS secids_isin64 secids_isin64_cli secids_cusip64_cli secids_sedol64_cli secids_figi64_cli secids_ric64_cli
    EXPORT secidsTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT secidsTargets
    FILE secidsTargets.cmake
    NAMESPACE secids::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/secids
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/secidsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/secidsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/secidsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/secids
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/secidsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/secidsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/secids
)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "mrayva")
set(CPACK_PACKAGE_CONTACT "mrayva@users.noreply.github.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_GENERATOR "TGZ")
include(CPack)

if(BUILD_TESTING)
    add_executable(secids_isin64_test test/isin64_test.cpp)
    target_link_libraries(secids_isin64_test PRIVATE secids::isin64)
    add_test(NAME secids_isin64_test COMMAND secids_isin64_test)

    add_executable(secids_cusip64_test test/cusip64_test.cpp)
    target_link_libraries(secids_cusip64_test PRIVATE secids::isin64)
    add_test(NAME secids_cusip64_test COMMAND secids_cusip64_test)

    add_executable(secids_sedol64_test test/sedol64_test.cpp)
    target_link_libraries(secids_sedol64_test PRIVATE secids::isin64)
    add_test(NAME secids_sedol64_test COMMAND secids_sedol64_test)

    add_executable(secids_figi64_test test/figi64_test.cpp)
    target_link_libraries(secids_figi64_test PRIVATE secids::isin64)
    add_test(NAME secids_figi64_test COMMAND secids_figi64_test)

    add_executable(secids_ric64_test test/ric64_test.cpp)
    target_link_libraries(secids_ric64_test PRIVATE secids::isin64)
    add_test(NAME secids_ric64_test COMMAND secids_ric64_test)
endif()
