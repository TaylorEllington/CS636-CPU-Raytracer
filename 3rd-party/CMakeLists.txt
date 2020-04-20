add_library(CLI11 INTERFACE)
target_include_directories( CLI11 INTERFACE CLI11.hpp)

add_library(stb-image-write INTERFACE)
target_include_directories( stb-image-write INTERFACE stb_image_write.h)


add_library(header-only INTERFACE)
target_include_directories( header-only INTERFACE header-only/)
target_include_directories( header-only INTERFACE glm/glm/)