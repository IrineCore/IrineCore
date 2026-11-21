# =============================================================================
# IrineCore — src/cmake/CompilerFlags.cmake
# Cross-platform C/C++ compiler, linker, and ABI configuration (Windows & Linux)
# Supports 64-bit (x64) Servers on Windows/Linux and native x64 / Win32 Client & Launcher on Windows.
# =============================================================================
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(MSVC)
    add_compile_options(
        /MP
        /bigobj
        /EHa
        /Zc:wchar_t
        /Zc:forScope
        /Zc:strictStrings-
        /Zc:twoPhase-
        /wd4005 /wd4018 /wd4091 /wd4100 /wd4101 /wd4189 /wd4244 /wd4250 /wd4267
        /wd4302 /wd4311 /wd4312 /wd4477 /wd4482 /wd4800 /wd4819 /wd4838 /wd4996
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Ot>
        $<$<CONFIG:Release>:/GF>
        $<$<CONFIG:Release>:/Gy>
    )
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_compile_options($<$<CONFIG:Release>:/arch:SSE2>)
    endif()
    add_compile_definitions(
        WIN32
        _CRT_SECURE_NO_WARNINGS
        _CRT_NON_CONFORMING_SWPRINTFS
        _WINSOCK_DEPRECATED_NO_WARNINGS
        _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_compile_definitions(WIN64 _WIN64)
        link_directories("${CMAKE_SOURCE_DIR}/common/VoiceChat/DolbyAxon/IceLib64")
    else()
        link_directories("${CMAKE_SOURCE_DIR}/common/VoiceChat/DolbyAxon/IceLib32")
    endif()

    function(irine_apply_client_optimizations target_name)
        get_target_property(_tgt_type "${target_name}" TYPE)
        if(IRINE_CLIENT_LTO)
            target_compile_options("${target_name}" PRIVATE $<$<CONFIG:Release>:/GL>)
        endif()
        if(_tgt_type STREQUAL "EXECUTABLE")
            target_link_options("${target_name}" PRIVATE
                /DYNAMICBASE
                /NXCOMPAT
                $<$<CONFIG:Release>:/OPT:REF>
                $<$<CONFIG:Release>:/OPT:ICF>
                $<$<CONFIG:Release>:/INCREMENTAL:NO>
            )
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                target_link_options("${target_name}" PRIVATE /HIGHENTROPYVA)
            else()
                target_link_options("${target_name}" PRIVATE /LARGEADDRESSAWARE)
            endif()
            if(IRINE_CLIENT_LTO)
                target_link_options("${target_name}" PRIVATE $<$<CONFIG:Release>:/LTCG>)
            endif()
        elseif(_tgt_type STREQUAL "STATIC_LIBRARY" AND IRINE_CLIENT_LTO)
            set_property(TARGET "${target_name}" APPEND_STRING PROPERTY STATIC_LIBRARY_FLAGS_RELEASE " /LTCG")
        endif()
    endfunction()
elseif(UNIX)
    add_compile_options(-fshort-wchar -pthread -Wno-deprecated-declarations)
    add_link_options(-pthread)
endif()
