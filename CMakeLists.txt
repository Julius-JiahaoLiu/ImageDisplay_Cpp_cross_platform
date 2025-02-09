cmake_minimum_required(VERSION 3.10)

project(ImageDisplay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
set(SOURCES
    src/Main.cpp
    manifest/MyImageApplication.rc
)

if(WIN32)
    set(wxBUILD_SHARED OFF)
    # or you can set it to ON, but you may face some problems about can't find dlls, in this case, you can copy dlls to your exe directory
else()
    set(wxBUILD_SHARED ON)
endif()

add_subdirectory(dependency/wxWidgets)
# find_package(wxWidgets REQUIRED)
# include(${wxWidgets_USE_FILE})

# Create executable
add_executable(MyImageApplication ${SOURCES})

# X11 required on Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# Add compile options for macOS
if(APPLE)
    target_compile_options(MyImageApplication PRIVATE -mmacosx-version-min=10.7 -stdlib=libc++)
endif()

# Link libraries
target_link_libraries(MyImageApplication wx::core wx::base ${X11_LIBRARIES})
# target_link_libraries(MyImageApplication ${wxWidgets_LIBRARIES})