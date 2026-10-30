cmake_minimum_required(VERSION 3.16)

project(lingmo-animation
    VERSION 5.0.0
    DESCRIPTION "Animation primitives for the Lingmo desktop"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Header only: the primitives are small, inline, and every consumer wants its
# own copy inlined into its frame loop.
add_library(lingmo-animation INTERFACE)
add_library(Lingmo::Animation ALIAS lingmo-animation)
set_target_properties(lingmo-animation PROPERTIES EXPORT_NAME Animation)
target_compile_features(lingmo-animation INTERFACE cxx_std_20)
target_include_directories(lingmo-animation INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(DIRECTORY include/lingmo DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS lingmo-animation EXPORT LingmoAnimationTargets)
install(EXPORT LingmoAnimationTargets
    FILE LingmoAnimationTargets.cmake
    NAMESPACE Lingmo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LingmoAnimation)

configure_package_config_file(cmake/LingmoAnimationConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LingmoAnimationConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LingmoAnimation)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LingmoAnimationConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LingmoAnimationConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LingmoAnimationConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LingmoAnimation)

configure_file(cmake/lingmo-animation.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/lingmo-animation.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-animation.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
