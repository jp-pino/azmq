project(test_message)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} Azmq::azmq Boost::boost ${Boost_SYSTEM_LIBRARY})

add_catch_test(${PROJECT_NAME})
