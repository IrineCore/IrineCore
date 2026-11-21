# IrineCore — deps/tinyxml/CMakeLists.txt
add_library(dep_tinyxml STATIC
    "XMLParser.cpp"
    "tinystr.cpp"
    "tinyxml.cpp"
    "tinyxmlerror.cpp"
    "tinyxmlparser.cpp"
)
target_include_directories(dep_tinyxml PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" ${IRINE_COMMON_INCLUDES})
target_compile_definitions(dep_tinyxml PRIVATE WIN64 NDEBUG _LIB TIXML_USE_STL _MBCS)
