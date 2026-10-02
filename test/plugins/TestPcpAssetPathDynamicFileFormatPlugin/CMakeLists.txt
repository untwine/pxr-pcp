add_library(TestPcpAssetPathDynamicFileFormatPlugin SHARED plugin.cpp)
target_link_libraries(TestPcpAssetPathDynamicFileFormatPlugin PUBLIC pcp pxr::sdf pxr::tf)
file(
    GENERATE
    OUTPUT "$<TARGET_FILE_DIR:TestPcpAssetPathDynamicFileFormatPlugin>/plugInfo_$<CONFIG>.json"
    INPUT plugInfo.json
)
