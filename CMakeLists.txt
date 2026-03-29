cmake_minimum_required(VERSION 3.20)

project(
    secids
    VERSION 0.1.0
    DESCRIPTION "Header-only C++20 market identifier packing and validation library"
    HOMEPAGE_URL "https://github.com/mrayva/secids"
    LANGUAGES CXX
)

option(SECIDS_WITH_OPENFIGI_VOCAB "Build optional string_bimap-backed OpenFIGI vocab support" OFF)
set(SECIDS_STRING_BIMAP_ROOT "" CACHE PATH "Optional path to a local string_bimap checkout")

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
find_package(Python3 COMPONENTS Interpreter QUIET)
find_program(BASH_EXECUTABLE bash)

add_library(secids_headers INTERFACE)
add_library(secids::secids ALIAS secids_headers)
set_target_properties(secids_headers PROPERTIES EXPORT_NAME secids)

target_include_directories(secids_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(secids_headers INTERFACE cxx_std_20)

if(SECIDS_WITH_OPENFIGI_VOCAB)
    if(SECIDS_STRING_BIMAP_ROOT)
        set(STRING_BIMAP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(STRING_BIMAP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(STRING_BIMAP_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
        add_subdirectory(${SECIDS_STRING_BIMAP_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/string_bimap EXCLUDE_FROM_ALL)
    else()
        find_package(string_bimap CONFIG QUIET)
    endif()

    if(NOT TARGET string_bimap::string_bimap)
        message(FATAL_ERROR "SECIDS_WITH_OPENFIGI_VOCAB=ON requires string_bimap. Set SECIDS_STRING_BIMAP_ROOT or install string_bimap.")
    endif()

    add_library(secids_openfigi_vocab INTERFACE)
    add_library(secids::openfigi_vocab ALIAS secids_openfigi_vocab)
    set_target_properties(secids_openfigi_vocab PROPERTIES EXPORT_NAME openfigi_vocab)
    target_link_libraries(secids_openfigi_vocab INTERFACE secids_headers string_bimap::string_bimap)
endif()

if(Python3_Interpreter_FOUND)
    add_custom_target(
        generate_iso3166_country_header
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_iso3166_country_header.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating ISO-3166 country header from vendored CSV"
    )

    add_custom_target(
        generate_iso4217_currency_header
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_iso4217_currency_header.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating ISO-4217 currency header from vendored CSV"
    )
endif()

if(BASH_EXECUTABLE)
    add_custom_target(
        fetch_iso_sources
        COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_iso_sources.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Fetching vendored ISO source data from upstream"
    )

    if(Python3_Interpreter_FOUND)
        add_custom_target(
            update_iso_sources
            COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/update_iso_sources.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Fetching upstream ISO source data and regenerating headers"
        )
    endif()

    add_custom_target(
        fetch_openfigi_values
        COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_openfigi_values.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Fetching OpenFIGI mapping value domains"
    )
endif()

add_library(secids_isin64 INTERFACE)
add_library(secids::isin64 ALIAS secids_isin64)
set_target_properties(secids_isin64 PROPERTIES EXPORT_NAME isin64)
target_link_libraries(secids_isin64 INTERFACE secids_headers)

add_executable(secids_isin64_cli src/isin64_cli.cpp)
target_link_libraries(secids_isin64_cli PRIVATE secids::secids)
target_compile_features(secids_isin64_cli PRIVATE cxx_std_20)

add_executable(secids_cusip64_cli src/cusip64_cli.cpp)
target_link_libraries(secids_cusip64_cli PRIVATE secids::secids)
target_compile_features(secids_cusip64_cli PRIVATE cxx_std_20)

add_executable(secids_sedol64_cli src/sedol64_cli.cpp)
target_link_libraries(secids_sedol64_cli PRIVATE secids::secids)
target_compile_features(secids_sedol64_cli PRIVATE cxx_std_20)

add_executable(secids_figi64_cli src/figi64_cli.cpp)
target_link_libraries(secids_figi64_cli PRIVATE secids::secids)
target_compile_features(secids_figi64_cli PRIVATE cxx_std_20)

add_executable(secids_ric64_cli src/ric64_cli.cpp)
target_link_libraries(secids_ric64_cli PRIVATE secids::secids)
target_compile_features(secids_ric64_cli PRIVATE cxx_std_20)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS secids_headers secids_isin64 secids_isin64_cli secids_cusip64_cli secids_sedol64_cli secids_figi64_cli secids_ric64_cli
    EXPORT secidsTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(TARGET secids_openfigi_vocab)
    add_executable(secids_openfigi_vocab_cli src/openfigi_vocab_cli.cpp)
    target_link_libraries(secids_openfigi_vocab_cli PRIVATE secids::openfigi_vocab)
    target_compile_features(secids_openfigi_vocab_cli PRIVATE cxx_std_20)

    install(
        TARGETS secids_openfigi_vocab secids_openfigi_vocab_cli
        EXPORT secidsTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

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
    target_link_libraries(secids_isin64_test PRIVATE secids::secids)
    add_test(NAME secids_isin64_test COMMAND secids_isin64_test)

    add_executable(secids_cusip64_test test/cusip64_test.cpp)
    target_link_libraries(secids_cusip64_test PRIVATE secids::secids)
    add_test(NAME secids_cusip64_test COMMAND secids_cusip64_test)

    add_executable(secids_sedol64_test test/sedol64_test.cpp)
    target_link_libraries(secids_sedol64_test PRIVATE secids::secids)
    add_test(NAME secids_sedol64_test COMMAND secids_sedol64_test)

    add_executable(secids_figi64_test test/figi64_test.cpp)
    target_link_libraries(secids_figi64_test PRIVATE secids::secids)
    add_test(NAME secids_figi64_test COMMAND secids_figi64_test)

    add_executable(secids_ric64_test test/ric64_test.cpp)
    target_link_libraries(secids_ric64_test PRIVATE secids::secids)
    add_test(NAME secids_ric64_test COMMAND secids_ric64_test)

    add_executable(secids_mic32_test test/mic32_test.cpp)
    target_link_libraries(secids_mic32_test PRIVATE secids::secids)
    add_test(NAME secids_mic32_test COMMAND secids_mic32_test)

    add_executable(secids_iso3166_country_test test/iso3166_country_test.cpp)
    target_link_libraries(secids_iso3166_country_test PRIVATE secids::secids)
    add_test(NAME secids_iso3166_country_test COMMAND secids_iso3166_country_test)

    add_executable(secids_iso4217_currency_test test/iso4217_currency_test.cpp)
    target_link_libraries(secids_iso4217_currency_test PRIVATE secids::secids)
    add_test(NAME secids_iso4217_currency_test COMMAND secids_iso4217_currency_test)

    add_executable(secids_iso_data_loader_test test/iso_data_loader_test.cpp)
    target_link_libraries(secids_iso_data_loader_test PRIVATE secids::secids)
    add_test(NAME secids_iso_data_loader_test COMMAND secids_iso_data_loader_test)

    if(TARGET secids_openfigi_vocab)
        add_executable(secids_openfigi_vocab_test test/openfigi_vocab_test.cpp)
        target_link_libraries(secids_openfigi_vocab_test PRIVATE secids::openfigi_vocab)
        add_test(NAME secids_openfigi_vocab_test COMMAND secids_openfigi_vocab_test)
    endif()
endif()
