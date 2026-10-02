add_library(TestPcpDynamicFileFormatPlugin SHARED plugin.cpp)
target_link_libraries(TestPcpDynamicFileFormatPlugin PUBLIC pcp pxr::sdf pxr::tf)
file(
    GENERATE
    OUTPUT "$<TARGET_FILE_DIR:TestPcpDynamicFileFormatPlugin>/plugInfo_$<CONFIG>.json"
    INPUT plugInfo.json
)
