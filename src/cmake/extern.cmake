FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG        master
  GIT_PROGRESS   TRUE
)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG        develop
  GIT_PROGRESS   TRUE
)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(argparse json googletest)

FetchContent_Declare(
  stb_repo
  GIT_REPOSITORY https://github.com/nothings/stb
  GIT_TAG master
)
FetchContent_GetProperties(stb_repo)
if(NOT stb_repo_POPULATED)
    FetchContent_Populate(stb_repo)
endif()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_repo_SOURCE_DIR})
add_library(stb::stb ALIAS stb)

