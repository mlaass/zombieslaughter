cmake_minimum_required(VERSION 3.16)
project(ZombieSlaughter C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allegro 4 (Ubuntu: liballegro4-dev -> -lalleg, headers in /usr/include/allegro)
find_library(ALLEG_LIB NAMES alleg alleg-4.4 REQUIRED)
find_path(ALLEG_INC allegro.h REQUIRED)

# --- game -------------------------------------------------------------------
file(GLOB GAME_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(zombieslaughter ${GAME_SRC} third_party/ppcol/ppcol.c)
target_include_directories(zombieslaughter PRIVATE
    src third_party/ppcol third_party/fblend ${ALLEG_INC})
target_link_libraries(zombieslaughter ${ALLEG_LIB} m)

# the game calls load_datafile("data.dat") relative to its CWD; copy it beside
# the binary so ./zombieslaughter just works from the build dir
add_custom_command(TARGET zombieslaughter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/original/ZombieSlaughter/data.dat
        $<TARGET_FILE_DIR:zombieslaughter>/data.dat)

# --- asset extractor tool ---------------------------------------------------
add_executable(dat_extract tools/dat_extract.c)
target_include_directories(dat_extract PRIVATE ${ALLEG_INC})
target_link_libraries(dat_extract ${ALLEG_LIB} m)
