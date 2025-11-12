cmake_minimum_required(VERSION 3.10)
project(ContextLauncher)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add executable
add_executable(launcher WIN32 launcher.cpp)

# Link required Windows libraries
target_link_libraries(launcher
    ole32
    oleaut32
    shlwapi
    shell32
    user32
)

# Set additional compile options for release builds
if(MSVC)
    target_compile_options(launcher PRIVATE
        $<$<CONFIG:Release>:/O2 /GL>
    )
    target_link_options(launcher PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
endif()

# Copy config file to output directory after build
add_custom_command(TARGET launcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/launcher.ini
        $<TARGET_FILE_DIR:launcher>/launcher.ini
    COMMENT "Copying launcher.ini to output directory"
)

# Installation rules
install(TARGETS launcher DESTINATION bin)
install(FILES launcher.ini DESTINATION bin)
install(FILES config-editor.html DESTINATION bin)
install(FILES README.md DESTINATION .)
