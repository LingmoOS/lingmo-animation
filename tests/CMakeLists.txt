add_executable(lingmo-animation-test animation_test.cpp)
target_link_libraries(lingmo-animation-test PRIVATE Lingmo::Animation)
add_test(NAME lingmo-animation COMMAND lingmo-animation-test)
