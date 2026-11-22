find_package(doctest CONFIG REQUIRED)

# The fixed step alone: pure computation, no module, no clock, so `unit`, and it runs in CI.
add_executable(test_framework_fixed_step_cxx test_framework_fixed_step_cxx.cpp)
liara_configure_target(test_framework_fixed_step_cxx)
target_link_libraries(test_framework_fixed_step_cxx PRIVATE
        Liara::Framework
        doctest::doctest)
add_test(NAME test_framework_fixed_step_cxx COMMAND test_framework_fixed_step_cxx)
set_tests_properties(test_framework_fixed_step_cxx PROPERTIES LABELS "unit")
