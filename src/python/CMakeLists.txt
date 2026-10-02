add_library(pyPcp SHARED
    module.cpp
    wrapCache.cpp
    wrapDependency.cpp
    wrapDynamicFileFormatDependencyData.cpp
    wrapErrors.cpp
    wrapExpressionVariables.cpp
    wrapExpressionVariablesSource.cpp
    wrapInstanceKey.cpp
    wrapPrimIndex.cpp
    wrapLayerRelocatesEditBuilder.cpp
    wrapLayerStack.cpp
    wrapLayerStackIdentifier.cpp
    wrapMapExpression.cpp
    wrapMapFunction.cpp
    wrapNode.cpp
    wrapPathTranslation.cpp
    wrapPropertyIndex.cpp
    wrapSite.cpp
    wrapTestChangeProcessor.cpp
    wrapTypes.cpp
)

set_target_properties(pyPcp
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME _pcp
        RUNTIME_OUTPUT_DIRECTORY "$<CONFIG>/pxr/Pcp"
        LIBRARY_OUTPUT_DIRECTORY "$<CONFIG>/pxr/Pcp"
        ARCHIVE_OUTPUT_DIRECTORY "$<CONFIG>/pxr/Pcp"
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(pyPcp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(pyPcp
    PUBLIC
        pcp
        pxr::boost-python
)

if(NOT WIN32)
    set(_rpath_prefix "$ORIGIN")
    if(APPLE)
        set(_rpath_prefix "@loader_path")
    endif()

    if(SKBUILD)
        set(_pyPcp_rpath "${_rpath_prefix}/../.libs")
    else()
        file(RELATIVE_PATH _pyPcp_libdir_rel
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_PYTHON_LIBDIR}/pxr/Pcp"
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
        )
        set(_pyPcp_rpath "${_rpath_prefix}/${_pyPcp_libdir_rel}")
    endif()

    set_target_properties(pyPcp PROPERTIES
        INSTALL_RPATH "${_pyPcp_rpath}"
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

if(WIN32)
    set_target_properties(pyPcp PROPERTIES SUFFIX ".pyd")
elseif(APPLE)
    set_target_properties(pyPcp PROPERTIES SUFFIX ".so")
endif()

if(MSVC)
    target_compile_options(pyPcp PRIVATE /bigobj)
endif()

target_compile_definitions(
    pyPcp
    PRIVATE
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        BOOST_PYTHON_NO_PY_SIGNATURES
        MFB_PACKAGE_MODULE=Pcp
        MFB_PACKAGE_NAME=pcp
        MFB_ALT_PACKAGE_NAME=pcp
)

file(
    GENERATE
    OUTPUT "$<TARGET_FILE_DIR:pyPcp>/__init__.py"
    INPUT "__init__.py"
)

install(
    TARGETS pyPcp EXPORT ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PYTHON_LIBDIR}/pxr/Pcp
    RUNTIME DESTINATION ${CMAKE_INSTALL_PYTHON_LIBDIR}/pxr/Pcp
)
install(
    FILES __init__.py
    DESTINATION ${CMAKE_INSTALL_PYTHON_LIBDIR}/pxr/Pcp
)
