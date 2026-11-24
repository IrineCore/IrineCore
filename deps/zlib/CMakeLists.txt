# IrineCore — deps/zlib/CMakeLists.txt
add_library(dep_zlib STATIC
    "adler32.c"
    "compress.c"
    "crc32.c"
    "deflate.c"
    "example.c"
    "gzio.c"
    "infback.c"
    "inffast.c"
    "inflate.c"
    "inftrees.c"
    "minigzip.c"
    "trees.c"
    "uncompr.c"
    "zutil.c"
)
target_include_directories(dep_zlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(dep_zlib PRIVATE $<$<BOOL:${WIN32}>:WIN64> NDEBUG _CRT_SECURE_NO_WARNINGS)
