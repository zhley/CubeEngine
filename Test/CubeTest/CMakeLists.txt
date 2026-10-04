add_executable(01_reflection src/01_reflection.cpp)
target_link_libraries(01_reflection PRIVATE Cube)

add_executable(02_script src/02_script.cpp)
target_link_libraries(02_script PRIVATE Cube)

add_executable(03_physics src/03_physics.cpp)
target_link_libraries(03_physics PRIVATE Cube)
