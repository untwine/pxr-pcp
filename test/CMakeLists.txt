if (WIN32)
    # Patch CMake's 'add_test' to extend the DLL search path on Windows. Use the
    # target name associated with the test, if it is also a CMake target.

    macro(add_test)
        _add_test(${ARGV})

        cmake_parse_arguments("" "" "NAME" "" ${ARGV})
        if (TARGET ${_NAME})
            set(DLL_DIRS $<TARGET_RUNTIME_DLL_DIRS:${_NAME}>)
        else()
            set(DLL_DIRS $<TARGET_RUNTIME_DLL_DIRS:pyPcp>)
        endif()
        set(DLL_DIRS
            ${DLL_DIRS}
            "${onetbb_LIB_DIRS_RELEASE}"
            "${onetbb_BIN_DIRS_RELEASE}"
        )
        set_tests_properties(${_NAME}
            PROPERTIES ENVIRONMENT
            "PATH=$<JOIN:$<SHELL_PATH:${DLL_DIRS}>,\;>")
    endmacro()
endif()

add_subdirectory(plugins)

# Test scripts are laid out per configuration, mirroring upstream's installed
# 'tests' directory.
set(_testDir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

set(PATH_SEP $<IF:$<BOOL:${WIN32}>,$<SEMICOLON>,:>)

set(_plugins
    "$<TARGET_FILE_DIR:pxr::sdf>/usd/sdf/resources/plugInfo.json"
    "$<TARGET_FILE_DIR:pxr::ar>/usd/ar/resources/plugInfo.json"
)

set(_pythonPath
    "$<TARGET_FILE_DIR:pxr::pyTf>/../.."
    "$<TARGET_FILE_DIR:pxr::pyGf>/../.."
    "$<TARGET_FILE_DIR:pxr::pyTrace>/../.."
    "$<TARGET_FILE_DIR:pxr::pyTs>/../.."
    "$<TARGET_FILE_DIR:pxr::pyVt>/../.."
    "$<TARGET_FILE_DIR:pxr::pyWork>/../.."
    "$<TARGET_FILE_DIR:pxr::pyPlug>/../.."
    "$<TARGET_FILE_DIR:pxr::pyAr>/../.."
    "$<TARGET_FILE_DIR:pxr::pySdf>/../.."
    "$<TARGET_FILE_DIR:pyPcp>/../.."
)

list(JOIN _plugins "${PATH_SEP}" _plugins)
list(JOIN _pythonPath "${PATH_SEP}" _pythonPath)

set(testWrapper
    "${Python_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/testWrapper.py"
    "--env-var=PXR_PLUGINPATH_NAME=${_plugins}"
    # Enable self-consistency checks for all tests in this library.
    "--env-var=PCP_ENABLE_CONSISTENCY_CHECKS=1"
)

add_executable(testPcpHardToReach testPcpHardToReach.cpp)
target_link_libraries(testPcpHardToReach PUBLIC pcp pxr::sdf pxr::tf)
add_test(NAME testPcpHardToReach
    COMMAND ${testWrapper}
        "$<TARGET_FILE:testPcpHardToReach>"
)

add_executable(testPcpIterator testPcpIterator.cpp)
target_link_libraries(testPcpIterator PUBLIC pcp pxr::sdf pxr::tf)
add_test(NAME testPcpIterator
    COMMAND ${testWrapper}
        "$<TARGET_FILE:testPcpIterator>"
        --testenv-dir=testPcpIterator.testenv
        --diff-compare=iteration_results.txt
)

add_executable(testPcpMapExpression testPcpMapExpression.cpp)
target_link_libraries(testPcpMapExpression PUBLIC pcp pxr::sdf pxr::tf)
add_test(NAME testPcpMapExpression
    COMMAND ${testWrapper}
        "$<TARGET_FILE:testPcpMapExpression>"
)

add_executable(testPcpPathTranslation_HardToReach testPcpPathTranslation_HardToReach.cpp)
target_link_libraries(testPcpPathTranslation_HardToReach PUBLIC pcp pxr::sdf pxr::tf)
add_test(NAME testPcpPathTranslation_HardToReach
    COMMAND ${testWrapper}
        "$<TARGET_FILE:testPcpPathTranslation_HardToReach>"
        --testenv-dir=testPcpPathTranslation_HardToReach.testenv
)

if(BUILD_PYTHON_BINDINGS)
    set(_scripts
        testPcpCache.py
        testPcpChanges.py
        testPcpExpressionComposition.py
        testPcpExpressionVariables.py
        testPcpExpressionVariablesSource.py
        testPcpInstanceKey.py
        testPcpLayerMuting.py
        testPcpDependencies.py
        testPcpPathTranslation.py
        testPcpPrimIndex.py
        testPcpDynamicFileFormatPlugin.py
        testPcpMapFunction.py
        testPcpOwner.py
        testPcpRegressionBugs_bug70951.py
        testPcpRegressionBugs_bug82180.py
        testPcpRegressionBugs_bug90508.py
        testPcpRegressionBugs_bug90706.py
        testPcpRegressionBugs_bug92955.py
        testPcpRegressionBugs_bug101300.py
        testPcpLayerRelocatesEditBuilder.py
        testPcpResolvedPathChange.py
        testPcpStreamingLayerReload.py
        testPcpCompositionResults.py
        testPcpNegativeOffsetScale.py
    )
    foreach(_script ${_scripts})
        file(GENERATE OUTPUT "${_testDir}/${_script}" INPUT "${_script}")
    endforeach()

    list(APPEND testWrapper
        "--env-var=PYTHONPATH=${_pythonPath}"
    )

    add_test(NAME testPcpMuseum_BasicInstancing
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicInstancing/root.usda"
            --testenv-dir=testPcpMuseum_BasicInstancing.testenv
            --diff-stdout=compositionResults_BasicInstancing.txt
            --env-var=PCP_OVERRIDE_INSTANCEABLE=1
    )

    add_test(NAME testPcpMuseum_BasicInstancingAndNestedInstances
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicInstancingAndNestedInstances/root.usda"
            --testenv-dir=testPcpMuseum_BasicInstancingAndNestedInstances.testenv
            --diff-stdout=compositionResults_BasicInstancingAndNestedInstances.txt
            --env-var=PCP_OVERRIDE_INSTANCEABLE=1
    )

    add_test(NAME testPcpMuseum_BasicInstancingAndVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicInstancingAndVariants/root.usda"
            --testenv-dir=testPcpMuseum_BasicInstancingAndVariants.testenv
            --diff-stdout=compositionResults_BasicInstancingAndVariants.txt
            --env-var=PCP_OVERRIDE_INSTANCEABLE=1
    )

    add_test(NAME testPcpMuseum_BasicPayloadDiamond
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --payloads ^(?!/RootUnloaded) BasicPayloadDiamond/root.usda"
            --testenv-dir=testPcpMuseum_BasicPayloadDiamond.testenv
            --diff-stdout=compositionResults_BasicPayloadDiamond.txt
            --env-var=PCP_OVERRIDE_INSTANCEABLE=1
    )

    add_test(NAME testPcpMuseum_BasicReference
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicReference/root.usda --session BasicReference/session.usda"
            --testenv-dir=testPcpMuseum_BasicReference.testenv
            --diff-stdout=compositionResults_BasicReference.txt
    )

    add_test(NAME testPcpMuseum_BasicReferenceAndClass
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicReferenceAndClass/root.usda"
            --testenv-dir=testPcpMuseum_BasicReferenceAndClass.testenv
            --diff-stdout=compositionResults_BasicReferenceAndClass.txt
    )

    add_test(NAME testPcpMuseum_BasicReferenceAndClassDiamond
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicReferenceAndClassDiamond/root.usda"
            --testenv-dir=testPcpMuseum_BasicReferenceAndClassDiamond.testenv
            --diff-stdout=compositionResults_BasicReferenceAndClassDiamond.txt
    )

    add_test(NAME testPcpMuseum_BasicAncestralReference
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicAncestralReference/root.usda"
            --testenv-dir=testPcpMuseum_BasicAncestralReference.testenv
            --diff-stdout=compositionResults_BasicAncestralReference.txt
    )

    add_test(NAME testPcpMuseum_BasicReferenceDiamond
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicReferenceDiamond/root.usda"
            --testenv-dir=testPcpMuseum_BasicReferenceDiamond.testenv
            --diff-stdout=compositionResults_BasicReferenceDiamond.txt
    )

    add_test(NAME testPcpMuseum_BasicRelocateToAnimInterface
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicRelocateToAnimInterface/root.usda"
            --testenv-dir=testPcpMuseum_BasicRelocateToAnimInterface.testenv
            --diff-stdout=compositionResults_BasicRelocateToAnimInterface.txt
    )

    add_test(NAME testPcpMuseum_BasicRelocateToAnimInterfaceAsNewRootPrim
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_BasicRelocateToAnimInterfaceAsNewRootPrim_usd.txt BasicRelocateToAnimInterfaceAsNewRootPrim/root.usda"
            --testenv-dir=testPcpMuseum_BasicRelocateToAnimInterfaceAsNewRootPrim.testenv
            --diff-stdout=compositionResults_BasicRelocateToAnimInterfaceAsNewRootPrim_usd.txt
            --diff-compare=compositionErrors_BasicRelocateToAnimInterfaceAsNewRootPrim_usd.txt
    )

    add_test(NAME testPcpMuseum_BasicInherits
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_BasicInherits.txt BasicInherits/root.usda"
            --testenv-dir=testPcpMuseum_BasicInherits.testenv
            --diff-stdout=compositionResults_BasicInherits.txt
            --diff-compare=compositionErrors_BasicInherits.txt
            --env-var=SDF_SCHEMA_PROHIBIT_INVALID_VARIANT_SELECTIONS=0
    )

    add_test(NAME testPcpMuseum_BasicLocalAndGlobalClassCombination
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicLocalAndGlobalClassCombination/root.usda"
            --testenv-dir=testPcpMuseum_BasicLocalAndGlobalClassCombination.testenv
            --diff-stdout=compositionResults_BasicLocalAndGlobalClassCombination.txt
    )

    add_test(NAME testPcpMuseum_BasicListEditing
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicListEditing/root.usda"
            --testenv-dir=testPcpMuseum_BasicListEditing.testenv
            --diff-stdout=compositionResults_BasicListEditing_usd.txt
    )

    add_test(NAME testPcpMuseum_BasicListEditingWithInherits
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicListEditingWithInherits/root.usda"
            --testenv-dir=testPcpMuseum_BasicListEditingWithInherits.testenv
            --diff-stdout=compositionResults_BasicListEditingWithInherits.txt
    )

    add_test(NAME testPcpMuseum_BasicDuplicateSublayer
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicDuplicateSublayer/root.usda"
            --testenv-dir=testPcpMuseum_BasicDuplicateSublayer.testenv
            --diff-stdout=compositionResults_BasicDuplicateSublayer.txt
    )

    add_test(NAME testPcpMuseum_BasicTimeOffset
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicTimeOffset/root.usda"
            --testenv-dir=testPcpMuseum_BasicTimeOffset.testenv
            --diff-stdout=compositionResults_BasicTimeOffset.txt
    )

    add_test(NAME testPcpMuseum_BasicNestedPayload
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicNestedPayload/root.usda"
            --testenv-dir=testPcpMuseum_BasicNestedPayload.testenv
            --diff-stdout=compositionResults_BasicNestedPayload.txt
    )

    add_test(NAME testPcpMuseum_BasicNestedVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicNestedVariants/root.usda"
            --testenv-dir=testPcpMuseum_BasicNestedVariants.testenv
            --diff-stdout=compositionResults_BasicNestedVariants.txt
    )

    add_test(NAME testPcpMuseum_BasicNestedVariantsWithSameName
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicNestedVariantsWithSameName/root.usda"
            --testenv-dir=testPcpMuseum_BasicNestedVariantsWithSameName.testenv
            --diff-stdout=compositionResults_BasicNestedVariantsWithSameName.txt
    )

    add_test(NAME testPcpMuseum_BasicVariantWithConnections
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicVariantWithConnections/root.usda"
            --testenv-dir=testPcpMuseum_BasicVariantWithConnections.testenv
            --diff-stdout=compositionResults_BasicVariantWithConnections.txt
    )

    add_test(NAME testPcpMuseum_BasicVariantWithReference
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_BasicVariantWithReference.txt BasicVariantWithReference/root.usda"
            --testenv-dir=testPcpMuseum_BasicVariantWithReference.testenv
            --diff-stdout=compositionResults_BasicVariantWithReference.txt
            --diff-compare=compositionErrors_BasicVariantWithReference.txt
    )

    add_test(NAME testPcpMuseum_BasicOwner
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --session BasicOwner/session.usda BasicOwner/root.usda"
            --testenv-dir=testPcpMuseum_BasicOwner.testenv
            --diff-stdout=compositionResults_BasicOwner.txt
    )

    add_test(NAME testPcpMuseum_BasicPayload
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --session BasicPayload/session.usda --errorFile compositionErrors_BasicPayload.txt BasicPayload/root.usda"
            --testenv-dir=testPcpMuseum_BasicPayload.testenv
            --diff-stdout=compositionResults_BasicPayload.txt
            --diff-compare=compositionErrors_BasicPayload.txt
    )

    add_test(NAME testPcpMuseum_BasicSpecializes
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicSpecializes/root.usda"
            --testenv-dir=testPcpMuseum_BasicSpecializes.testenv
            --diff-stdout=compositionResults_BasicSpecializes.txt
    )

    add_test(NAME testPcpMuseum_BasicSpecializesAndInherits
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicSpecializesAndInherits/root.usda"
            --testenv-dir=testPcpMuseum_BasicSpecializesAndInherits.testenv
            --diff-stdout=compositionResults_BasicSpecializesAndInherits.txt
    )

    add_test(NAME testPcpMuseum_BasicSpecializesAndReferences
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicSpecializesAndReferences/root.usda"
            --testenv-dir=testPcpMuseum_BasicSpecializesAndReferences.testenv
            --diff-stdout=compositionResults_BasicSpecializesAndReferences.txt
    )

    add_test(NAME testPcpMuseum_BasicSpecializesAndVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd BasicSpecializesAndVariants/root.usda"
            --testenv-dir=testPcpMuseum_BasicSpecializesAndVariants.testenv
            --diff-stdout=compositionResults_BasicSpecializesAndVariants.txt
    )

    add_test(NAME testPcpMuseum_EdgeCaseLocalAndGlobalClassCombination
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd EdgeCaseLocalAndGlobalClassCombination/root.usda"
            --testenv-dir=testPcpMuseum_EdgeCaseLocalAndGlobalClassCombination.testenv
            --diff-stdout=compositionResults_EdgeCaseLocalAndGlobalClassCombination.txt
    )

    add_test(NAME testPcpMuseum_ElidedAncestralRelocates
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ElidedAncestralRelocates/root.usda"
            --testenv-dir=testPcpMuseum_ElidedAncestralRelocates.testenv
            --diff-stdout=compositionResults_ElidedAncestralRelocates.txt
    )

    add_test(NAME testPcpMuseum_ExpressionsInPayloads
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ExpressionsInPayloads/root.usda"
            --testenv-dir=testPcpMuseum_ExpressionsInPayloads.testenv
            --diff-stdout=compositionResults_ExpressionsInPayloads.txt
    )

    add_test(NAME testPcpMuseum_ExpressionsInReferences
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ExpressionsInReferences/root.usda"
            --testenv-dir=testPcpMuseum_ExpressionsInReferences.testenv
            --diff-stdout=compositionResults_ExpressionsInReferences.txt
    )

    add_test(NAME testPcpMuseum_ExpressionsInSublayers
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ExpressionsInSublayers/root.usda --session ExpressionsInSublayers/session.usda --errorFile compositionErrors_ExpressionsInSublayers.txt"
            --testenv-dir=testPcpMuseum_ExpressionsInSublayers.testenv
            --diff-stdout=compositionResults_ExpressionsInSublayers.txt
            --diff-compare=compositionErrors_ExpressionsInSublayers.txt
    )

    add_test(NAME testPcpMuseum_ExpressionsInVariantSelections
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ExpressionsInVariantSelections/root.usda --errorFile compositionErrors_ExpressionsInVariantSelections.txt"
            --testenv-dir=testPcpMuseum_ExpressionsInVariantSelections.testenv
            --diff-stdout=compositionResults_ExpressionsInVariantSelections.txt
            --diff-compare=compositionErrors_ExpressionsInVariantSelections.txt
    )

    add_test(NAME testPcpMuseum_ImpliedAndAncestralInherits
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ImpliedAndAncestralInherits/root.usda"
            --testenv-dir=testPcpMuseum_ImpliedAndAncestralInherits.testenv
            --diff-stdout=compositionResults_ImpliedAndAncestralInherits.txt
    )

    add_test(NAME testPcpMuseum_ImpliedAndAncestralInherits_graph
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --dumpPath /World/CharGroup/Sim/includes/Hand/Geom --dumpMaps ImpliedAndAncestralInherits/root.usda"
            --testenv-dir=testPcpMuseum_ImpliedAndAncestralInherits.testenv
            --diff-stdout=compositionResults_ImpliedAndAncestralInherits_graph.txt
    )

    add_test(NAME testPcpMuseum_ImpliedArcsAndInstancing
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ImpliedArcsAndInstancing/root.usda"
            --testenv-dir=testPcpMuseum_ImpliedArcsAndInstancing.testenv
            --diff-stdout=compositionResults_ImpliedArcsAndInstancing.txt
    )

    add_test(NAME testPcpMuseum_InternalReferenceInOffsetSublayer
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd InternalReferenceInOffsetSublayer/root.usda"
            --testenv-dir=testPcpMuseum_InternalReferenceInOffsetSublayer.testenv
            --diff-stdout=compositionResults_InternalReferenceInOffsetSublayer.txt
    )

    add_test(NAME testPcpMuseum_ImpliedAndAncestralInherits_ComplexEvaluation
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ImpliedAndAncestralInherits_ComplexEvaluation/root.usda"
            --testenv-dir=testPcpMuseum_ImpliedAndAncestralInherits_ComplexEvaluation.testenv
            --diff-stdout=compositionResults_ImpliedAndAncestralInherits_ComplexEvaluation.txt
    )

    add_test(NAME testPcpMuseum_ListOpsWithLayerOffsets_prepend
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ListOpsWithLayerOffsets/base_prepend.usda"
            --testenv-dir=testPcpMuseum_ListOpsWithLayerOffsets.testenv
            --diff-stdout=compositionResults_ListOps_prepend.txt
    )

    add_test(NAME testPcpMuseum_ListOpsWithLayerOffsets_delete
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ListOpsWithLayerOffsets/base_delete.usda"
            --testenv-dir=testPcpMuseum_ListOpsWithLayerOffsets.testenv
            --diff-stdout=compositionResults_ListOps_delete.txt
    )

    add_test(NAME testPcpMuseum_ListOpsWithLayerOffsets_delete_24
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ListOpsWithLayerOffsets/base_delete_24.usda"
            --testenv-dir=testPcpMuseum_ListOpsWithLayerOffsets.testenv
            --diff-stdout=compositionResults_ListOps_delete_24.txt
    )

    add_test(NAME testPcpMuseum_ListOpsWithLayerOffsets_x2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ListOpsWithLayerOffsets/base_x2.usda"
            --testenv-dir=testPcpMuseum_ListOpsWithLayerOffsets.testenv
            --diff-stdout=compositionResults_ListOps_x2.txt
    )

    add_test(NAME testPcpMuseum_ListOpsWithLayerOffsets_sub_scaled
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ListOpsWithLayerOffsets/base_sub_scaled.usda"
            --testenv-dir=testPcpMuseum_ListOpsWithLayerOffsets.testenv
            --diff-stdout=compositionResults_ListOps_sub_scaled.txt
    )

    add_test(NAME testPcpMuseum_ListOpsWithLayerOffsets_multi_offset_sublayer
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ListOpsWithLayerOffsets/base_multi_offset_sublayer.usda"
            --testenv-dir=testPcpMuseum_ListOpsWithLayerOffsets.testenv
            --diff-stdout=compositionResults_ListOps_multi_offset_sublayer.txt
    )

    add_test(NAME testPcpMuseum_PayloadsAndAncestralArcs
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --payloads ^(?!/Root/UnloadedInheritedPayload) PayloadsAndAncestralArcs/root.usda"
            --testenv-dir=testPcpMuseum_PayloadsAndAncestralArcs.testenv
            --diff-stdout=compositionResults_PayloadsAndAncestralArcs.txt
    )

    add_test(NAME testPcpMuseum_PayloadsAndAncestralArcs2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd PayloadsAndAncestralArcs2/root.usda"
            --testenv-dir=testPcpMuseum_PayloadsAndAncestralArcs2.testenv
            --diff-stdout=compositionResults_PayloadsAndAncestralArcs2.txt
    )

    add_test(NAME testPcpMuseum_PayloadsAndAncestralArcs3_allpayloads
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd PayloadsAndAncestralArcs3/root.usda"
            --testenv-dir=testPcpMuseum_PayloadsAndAncestralArcs3.testenv
            --diff-stdout=compositionResults_PayloadsAndAncestralArcs3_allpayloads.txt
    )

    add_test(NAME testPcpMuseum_PayloadsAndAncestralArcs3_refpayloadsonly
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --payloads '.*Reference.*' PayloadsAndAncestralArcs3/root.usda"
            --testenv-dir=testPcpMuseum_PayloadsAndAncestralArcs3.testenv
            --diff-stdout=compositionResults_PayloadsAndAncestralArcs3_refpayloadsonly.txt
    )

    add_test(NAME testPcpMuseum_ReferenceListOpsWithOffsets
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd ReferenceListOpsWithOffsets/root.usda"
            --testenv-dir=testPcpMuseum_ReferenceListOpsWithOffsets.testenv
            --diff-stdout=compositionResults_ReferenceListOpsWithOffsets.txt
    )

    add_test(NAME testPcpMuseum_RelativePathPayloads
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd RelativePathPayloads/root.usda"
            --testenv-dir=testPcpMuseum_RelativePathPayloads.testenv
            --diff-stdout=compositionResults_RelativePathPayloads.txt
    )

    add_test(NAME testPcpMuseum_RelativePathReferences
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd RelativePathReferences/root.usda"
            --testenv-dir=testPcpMuseum_RelativePathReferences.testenv
            --diff-stdout=compositionResults_RelativePathReferences.txt
    )

    add_test(NAME testPcpMuseum_RelocatePrimsWithSameName
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_RelocatePrimsWithSameName.txt RelocatePrimsWithSameName/root.usda"
            --testenv-dir=testPcpMuseum_RelocatePrimsWithSameName.testenv
            --diff-stdout=compositionResults_RelocatePrimsWithSameName.txt
            --diff-compare=compositionErrors_RelocatePrimsWithSameName.txt
    )

    add_test(NAME testPcpMuseum_RelocateToNone
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_RelocateToNone_usd.txt RelocateToNone/root.usda"
            --testenv-dir=testPcpMuseum_RelocateToNone.testenv
            --diff-stdout=compositionResults_RelocateToNone_usd.txt
            --diff-compare=compositionErrors_RelocateToNone_usd.txt
    )
    add_test(NAME testPcpMuseum_SpecializesAndAncestralArcs
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndAncestralArcs/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndAncestralArcs.testenv
            --diff-stdout=compositionResults_SpecializesAndAncestralArcs.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndAncestralArcs2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndAncestralArcs2/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndAncestralArcs2.testenv
            --diff-stdout=compositionResults_SpecializesAndAncestralArcs2.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndAncestralArcs3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndAncestralArcs3/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndAncestralArcs3.testenv
            --diff-stdout=compositionResults_SpecializesAndAncestralArcs3.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndAncestralArcs4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndAncestralArcs4/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndAncestralArcs4.testenv
            --diff-stdout=compositionResults_SpecializesAndAncestralArcs4.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndAncestralArcs4_graph
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --dumpPath /Parent/Child SpecializesAndAncestralArcs4/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndAncestralArcs4.testenv
            --diff-stdout=compositionResults_SpecializesAndAncestralArcs4_graph.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndAncestralArcs5
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndAncestralArcs5/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndAncestralArcs5.testenv
            --diff-stdout=compositionResults_SpecializesAndAncestralArcs5.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndDuplicateArcs
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndDuplicateArcs/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndDuplicateArcs.testenv
            --diff-stdout=compositionResults_SpecializesAndDuplicateArcs.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndVariants/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndVariants.testenv
            --diff-stdout=compositionResults_SpecializesAndVariants.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndVariants2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndVariants2/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndVariants2.testenv
            --diff-stdout=compositionResults_SpecializesAndVariants2.txt
    )

    add_test(NAME testPcpMuseum_SpecializesAndVariants3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndVariants3/root.usda"
            --testenv-dir=testPcpMuseum_SpecializesAndVariants3.testenv
            --diff-stdout=compositionResults_SpecializesAndVariants3.txt
    )

    # XXX: Disabled pending fix for GitHub #3750
    #
    # add_test(NAME testPcpMuseum_SpecializesAndVariants4
    #     COMMAND ${testWrapper}
    #         "${_testDir}/testPcpCompositionResults.py --usd SpecializesAndVariants4/root.usda"
    #         --testenv-dir=testPcpMuseum_SpecializesAndVariants4.testenv
    #         --diff-stdout=compositionResults_SpecializesAndVariants4.txt
    # )

    add_test(NAME testPcpMuseum_SubrootInheritsAndVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SubrootInheritsAndVariants/root.usda"
            --testenv-dir=testPcpMuseum_SubrootInheritsAndVariants.testenv
            --diff-stdout=compositionResults_SubrootInheritsAndVariants.txt
    )

    add_test(NAME testPcpMuseum_SubrootReferenceAndClasses
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SubrootReferenceAndClasses/root.usda"
            --testenv-dir=testPcpMuseum_SubrootReferenceAndClasses.testenv
            --diff-stdout=compositionResults_SubrootReferenceAndClasses.txt
    )

    add_test(NAME testPcpMuseum_SubrootReferenceAndRelocates
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SubrootReferenceAndRelocates/root.usda"
            --testenv-dir=testPcpMuseum_SubrootReferenceAndRelocates.testenv
            --diff-stdout=compositionResults_SubrootReferenceAndRelocates.txt
    )

    add_test(NAME testPcpMuseum_SubrootReferenceAndVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_SubrootReferenceAndVariants.txt SubrootReferenceAndVariants/root.usda"
            --testenv-dir=testPcpMuseum_SubrootReferenceAndVariants.testenv
            --diff-stdout=compositionResults_SubrootReferenceAndVariants.txt
            --diff-compare=compositionErrors_SubrootReferenceAndVariants.txt
            --env-var=SDF_SCHEMA_PROHIBIT_INVALID_VARIANT_SELECTIONS=0
    )

    add_test(NAME testPcpMuseum_SubrootReferenceAndVariants2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd SubrootReferenceAndVariants2/root.usda"
            --testenv-dir=testPcpMuseum_SubrootReferenceAndVariants2.testenv
            --diff-stdout=compositionResults_SubrootReferenceAndVariants2.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecond1
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecond1.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecond2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_48tcps.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecond2.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecond3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_24tcps_12fps.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecond3.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecond4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_12fps.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecond4.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecondWithSessionLayer1
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_24tcps_12fps.usda --session TimeCodesPerSecond/session_48tcps.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecondWithSessionLayer1.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecondWithSessionLayer2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_48tcps.usda --session TimeCodesPerSecond/session_24fps.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecondWithSessionLayer2.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecondWithSessionLayer3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_12fps.usda --session TimeCodesPerSecond/session_24fps.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecondWithSessionLayer3.txt
    )

    add_test(NAME testPcpMuseum_TimeCodesPerSecondWithSessionLayer4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TimeCodesPerSecond/root_12fps.usda --session TimeCodesPerSecond/session.usda"
            --testenv-dir=testPcpMuseum_TimeCodesPerSecond.testenv
            --diff-stdout=compositionResults_TimeCodesPerSecondWithSessionLayer4.txt
    )

    add_test(NAME testPcpMuseum_TypicalReferenceToRiggedModel
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TypicalReferenceToRiggedModel/root.usda"
            --testenv-dir=testPcpMuseum_TypicalReferenceToRiggedModel.testenv
            --diff-stdout=compositionResults_TypicalReferenceToRiggedModel.txt
    )

    add_test(NAME testPcpMuseum_TypicalReferenceToChargroup
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TypicalReferenceToChargroup/root.usda"
            --testenv-dir=testPcpMuseum_TypicalReferenceToChargroup.testenv
            --diff-stdout=compositionResults_TypicalReferenceToChargroup.txt
    )

    add_test(NAME testPcpMuseum_TypicalReferenceToChargroupWithRename
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TypicalReferenceToChargroupWithRename/root.usda"
            --testenv-dir=testPcpMuseum_TypicalReferenceToChargroupWithRename.testenv
            --diff-stdout=compositionResults_TypicalReferenceToChargroupWithRename.txt
    )

    add_test(NAME testPcpMuseum_TrickyListEditedTargetPaths
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyListEditedTargetPaths/root.usda"
            --testenv-dir=testPcpMuseum_TrickyListEditedTargetPaths.testenv
            --diff-stdout=compositionResults_TrickyListEditedTargetPaths.txt
    )

    add_test(NAME testPcpMuseum_TrickySpookyInherits
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpookyInherits/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpookyInherits.testenv
            --diff-stdout=compositionResults_TrickySpookyInherits.txt
    )

    add_test(NAME testPcpMuseum_TrickySpookyInheritsInSymmetricArmRig
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpookyInheritsInSymmetricArmRig/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpookyInheritsInSymmetricArmRig.testenv
            --diff-stdout=compositionResults_TrickySpookyInheritsInSymmetricArmRig.txt
    )

    add_test(NAME testPcpMuseum_TrickySpookyInheritsInSymmetricBrowRig
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpookyInheritsInSymmetricBrowRig/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpookyInheritsInSymmetricBrowRig.testenv
            --diff-stdout=compositionResults_TrickySpookyInheritsInSymmetricBrowRig.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsAndRelocates
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd  --errorFile compositionErrors_TrickyInheritsAndRelocates.txt TrickyInheritsAndRelocates/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsAndRelocates.testenv
            --diff-stdout=compositionResults_TrickyInheritsAndRelocates.txt
            --diff-compare=compositionErrors_TrickyInheritsAndRelocates.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsAndRelocates2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyInheritsAndRelocates2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsAndRelocates2.testenv
            --diff-stdout=compositionResults_TrickyInheritsAndRelocates2.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsAndRelocates3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyInheritsAndRelocates3/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsAndRelocates3.testenv
            --diff-stdout=compositionResults_TrickyInheritsAndRelocates3.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsAndRelocates4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyInheritsAndRelocates4/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsAndRelocates4.testenv
            --diff-stdout=compositionResults_TrickyInheritsAndRelocates4.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsAndRelocates5
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyInheritsAndRelocates5/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsAndRelocates5.testenv
            --diff-stdout=compositionResults_TrickyInheritsAndRelocates5.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsAndRelocatesToNewRootPrim
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd  --errorFile compositionErrors_TrickyInheritsAndRelocatesToNewRootPrim.txt TrickyInheritsAndRelocatesToNewRootPrim/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsAndRelocatesToNewRootPrim.testenv
            --diff-stdout=compositionResults_TrickyInheritsAndRelocatesToNewRootPrim.txt
            --diff-compare=compositionErrors_TrickyInheritsAndRelocatesToNewRootPrim.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsInVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyInheritsInVariants/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsInVariants.testenv
            --diff-stdout=compositionResults_TrickyInheritsInVariants.txt
    )

    add_test(NAME testPcpMuseum_TrickyInheritsInVariants2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyInheritsInVariants2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyInheritsInVariants2.testenv
            --diff-stdout=compositionResults_TrickyInheritsInVariants2.txt
    )

    add_test(NAME testPcpMuseum_SubrootReferenceNonCycle
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_SubrootReferenceNonCycle.txt SubrootReferenceNonCycle/root.usda"
            --testenv-dir=testPcpMuseum_SubrootReferenceNonCycle.testenv
            --diff-stdout=compositionResults_SubrootReferenceNonCycle.txt
            --diff-compare=compositionErrors_SubrootReferenceNonCycle.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantAncestralSelection
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantAncestralSelection/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantAncestralSelection.testenv
            --diff-stdout=compositionResults_TrickyVariantAncestralSelection.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantIndependentSelection
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantIndependentSelection/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantIndependentSelection.testenv
            --diff-stdout=compositionResults_TrickyVariantIndependentSelection.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantInPayload
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantInPayload/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantInPayload.testenv
            --diff-stdout=compositionResults_TrickyVariantInPayload.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantOverrideOfLocalClass
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantOverrideOfLocalClass/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantOverrideOfLocalClass.testenv
            --diff-stdout=compositionResults_TrickyVariantOverrideOfLocalClass.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantOverrideOfRelocatedPrim
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantOverrideOfRelocatedPrim/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantOverrideOfRelocatedPrim.testenv
            --diff-stdout=compositionResults_TrickyVariantOverrideOfRelocatedPrim.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantSelectionInVariant
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantSelectionInVariant/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantSelectionInVariant.testenv
            --diff-stdout=compositionResults_TrickyVariantSelectionInVariant.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantSelectionInVariant2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantSelectionInVariant2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantSelectionInVariant2.testenv
            --diff-stdout=compositionResults_TrickyVariantSelectionInVariant2.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantWeakerSelection
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantWeakerSelection/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantWeakerSelection.testenv
            --diff-stdout=compositionResults_TrickyVariantWeakerSelection.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantWeakerSelection2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantWeakerSelection2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantWeakerSelection2.testenv
            --diff-stdout=compositionResults_TrickyVariantWeakerSelection2.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantWeakerSelection3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantWeakerSelection3/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantWeakerSelection3.testenv
            --diff-stdout=compositionResults_TrickyVariantWeakerSelection3.txt
    )

    add_test(NAME testPcpMuseum_TrickyVariantWeakerSelection4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyVariantWeakerSelection4/root.usda"
            --testenv-dir=testPcpMuseum_TrickyVariantWeakerSelection4.testenv
            --diff-stdout=compositionResults_TrickyVariantWeakerSelection4.txt
    )

    add_test(NAME testPcpMuseum_TrickyClassHierarchy
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyClassHierarchy/root.usda"
            --testenv-dir=testPcpMuseum_TrickyClassHierarchy.testenv
            --diff-stdout=compositionResults_TrickyClassHierarchy.txt
    )

    add_test(NAME testPcpMuseum_TrickyLocalClassHierarchyWithRelocates
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyLocalClassHierarchyWithRelocates/root.usda"
            --testenv-dir=testPcpMuseum_TrickyLocalClassHierarchyWithRelocates.testenv
            --diff-stdout=compositionResults_TrickyLocalClassHierarchyWithRelocates.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedClasses
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedClasses/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedClasses.testenv
            --diff-stdout=compositionResults_TrickyNestedClasses.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedClasses2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedClasses2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedClasses2.testenv
            --diff-stdout=compositionResults_TrickyNestedClasses2.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedClasses3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedClasses3/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedClasses3.testenv
            --diff-stdout=compositionResults_TrickyNestedClasses3.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedSpecializes
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedSpecializes/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedSpecializes.testenv
            --diff-stdout=compositionResults_TrickyNestedSpecializes.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedSpecializes2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedSpecializes2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedSpecializes2.testenv
            --diff-stdout=compositionResults_TrickyNestedSpecializes2.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedClasses4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedClasses4/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedClasses4.testenv
            --diff-stdout=compositionResults_TrickyNestedClasses4.txt
    )

    add_test(NAME testPcpMuseum_TrickyNestedVariants
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNestedVariants/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNestedVariants.testenv
            --diff-stdout=compositionResults_TrickyNestedVariants.txt
    )

    add_test(NAME testPcpMuseum_TrickySpecializesAndInherits
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpecializesAndInherits/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpecializesAndInherits.testenv
            --diff-stdout=compositionResults_TrickySpecializesAndInherits.txt
    )

    add_test(NAME testPcpMuseum_TrickySpecializesAndInherits2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpecializesAndInherits2/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpecializesAndInherits2.testenv
            --diff-stdout=compositionResults_TrickySpecializesAndInherits2.txt
    )

    add_test(NAME testPcpMuseum_TrickySpecializesAndInherits3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpecializesAndInherits3/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpecializesAndInherits3.testenv
            --diff-stdout=compositionResults_TrickySpecializesAndInherits3.txt
    )

    add_test(NAME testPcpMuseum_TrickySpecializesAndRelocates
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpecializesAndRelocates/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpecializesAndRelocates.testenv
            --diff-stdout=compositionResults_TrickySpecializesAndRelocates.txt
    )

    add_test(NAME testPcpMuseum_TrickySpookyVariantSelection
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpookyVariantSelection/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpookyVariantSelection.testenv
            --diff-stdout=compositionResults_TrickySpookyVariantSelection.txt
    )

    add_test(NAME testPcpMuseum_TrickySpookyVariantSelectionInClass
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickySpookyVariantSelectionInClass/root.usda"
            --testenv-dir=testPcpMuseum_TrickySpookyVariantSelectionInClass.testenv
            --diff-stdout=compositionResults_TrickySpookyVariantSelectionInClass.txt
    )

    add_test(NAME testPcpMuseum_TrickyConnectionToRelocatedAttribute
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyConnectionToRelocatedAttribute/root.usda"
            --testenv-dir=testPcpMuseum_TrickyConnectionToRelocatedAttribute.testenv
            --diff-stdout=compositionResults_TrickyConnectionToRelocatedAttribute.txt
    )

    add_test(NAME testPcpMuseum_TrickyRelocatedTargetInVariant
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyRelocatedTargetInVariant/root.usda"
            --testenv-dir=testPcpMuseum_TrickyRelocatedTargetInVariant.testenv
            --diff-stdout=compositionResults_TrickyRelocatedTargetInVariant.txt
    )

    add_test(NAME testPcpMuseum_TrickyRelocationOfPrimFromPayload
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyRelocationOfPrimFromPayload/root.usda"
            --testenv-dir=testPcpMuseum_TrickyRelocationOfPrimFromPayload.testenv
            --diff-stdout=compositionResults_TrickyRelocationOfPrimFromPayload.txt
    )

    add_test(NAME testPcpMuseum_TrickyRelocationOfPrimFromVariant
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyRelocationOfPrimFromVariant/root.usda"
            --testenv-dir=testPcpMuseum_TrickyRelocationOfPrimFromVariant.testenv
            --diff-stdout=compositionResults_TrickyRelocationOfPrimFromVariant.txt
    )

    add_test(NAME testPcpMuseum_TrickyRelocationSquatter
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyRelocationSquatter/root.usda"
            --testenv-dir=testPcpMuseum_TrickyRelocationSquatter.testenv
            --diff-stdout=compositionResults_TrickyRelocationSquatter.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocations
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocations/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocations.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocations.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocations2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocations2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocations2.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocations2.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocations3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocations3/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocations3.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocations3.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocations4
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocations4/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocations4.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocations4.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocations5
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocations5/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocations5.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocations5.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocationsAndClasses
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocationsAndClasses/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocationsAndClasses.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocationsAndClasses.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocationsAndClasses2
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocationsAndClasses2/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocationsAndClasses2.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocationsAndClasses2.txt
    )

    add_test(NAME testPcpMuseum_TrickyMultipleRelocationsAndClasses3
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyMultipleRelocationsAndClasses3/root.usda"
            --testenv-dir=testPcpMuseum_TrickyMultipleRelocationsAndClasses3.testenv
            --diff-stdout=compositionResults_TrickyMultipleRelocationsAndClasses3.txt
    )

    add_test(NAME testPcpMuseum_TrickyNonLocalVariantSelection
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd TrickyNonLocalVariantSelection/root.usda"
            --testenv-dir=testPcpMuseum_TrickyNonLocalVariantSelection.testenv
            --diff-stdout=compositionResults_TrickyNonLocalVariantSelection.txt
    )

    add_test(NAME testPcpMuseum_UnmappableImpliedInherit
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd UnmappableImpliedInherit/root.usda"
            --testenv-dir=testPcpMuseum_UnmappableImpliedInherit.testenv
            --diff-stdout=compositionResults_UnmappableImpliedInherit.txt
    )

    add_test(NAME testPcpMuseum_ErrorOpinionAtRelocationSource
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorOpinionAtRelocationSource.txt ErrorOpinionAtRelocationSource/root.usda"
            --testenv-dir=testPcpMuseum_ErrorOpinionAtRelocationSource.testenv
            --diff-stdout=compositionResults_ErrorOpinionAtRelocationSource.txt
            --diff-compare=compositionErrors_ErrorOpinionAtRelocationSource.txt
    )

    add_test(NAME testPcpMuseum_ErrorArcCycle
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorArcCycle.txt ErrorArcCycle/root.usda"
            --testenv-dir=testPcpMuseum_ErrorArcCycle.testenv
            --diff-stdout=compositionResults_ErrorArcCycle.txt
            --diff-compare=compositionErrors_ErrorArcCycle.txt
    )

    add_test(NAME testPcpMuseum_ErrorPermissionDenied
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd  ErrorPermissionDenied/root.usda"
            --testenv-dir=testPcpMuseum_ErrorPermissionDenied.testenv
            --diff-stdout=compositionResults_ErrorPermissionDenied_usd.txt
    )

    add_test(NAME testPcpMuseum_ErrorConnectionPermissionDenied
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd  ErrorConnectionPermissionDenied/root.usda"
            --testenv-dir=testPcpMuseum_ErrorConnectionPermissionDenied.testenv
            --diff-stdout=compositionResults_ErrorConnectionPermissionDenied_usd.txt
    )

    add_test(NAME testPcpMuseum_ErrorInconsistentProperties
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInconsistentProperties_usd.txt ErrorInconsistentProperties/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInconsistentProperties.testenv
            --diff-stdout=compositionResults_ErrorInconsistentProperties_usd.txt
            --diff-compare=compositionErrors_ErrorInconsistentProperties_usd.txt
    )

    add_test(NAME testPcpMuseum_ErrorInvalidInstanceTargetPath
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInvalidInstanceTargetPath_usd.txt ErrorInvalidInstanceTargetPath/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInvalidInstanceTargetPath.testenv
            --diff-stdout=compositionResults_ErrorInvalidInstanceTargetPath_usd.txt
            --diff-compare=compositionErrors_ErrorInvalidInstanceTargetPath_usd.txt
    )

    add_test(NAME testPcpMuseum_ErrorInvalidPreRelocateTargetPath
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd  ErrorInvalidPreRelocateTargetPath/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInvalidPreRelocateTargetPath.testenv
            --diff-stdout=compositionResults_ErrorInvalidPreRelocateTargetPath_usd.txt
    )

    add_test(NAME testPcpMuseum_ErrorInvalidTargetPath
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInvalidTargetPath.txt ErrorInvalidTargetPath/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInvalidTargetPath.testenv
            --diff-stdout=compositionResults_ErrorInvalidTargetPath.txt
            --diff-compare=compositionErrors_ErrorInvalidTargetPath.txt
    )

    add_test(NAME testPcpMuseum_ErrorInvalidPayload
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInvalidPayload.txt ErrorInvalidPayload/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInvalidPayload.testenv
            --diff-stdout=compositionResults_ErrorInvalidPayload.txt
            --diff-compare=compositionErrors_ErrorInvalidPayload.txt
    )

    add_test(NAME testPcpMuseum_ErrorSublayerCycle
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorSublayerCycle.txt ErrorSublayerCycle/root.usda"
            --testenv-dir=testPcpMuseum_ErrorSublayerCycle.testenv
            --diff-stdout=compositionResults_ErrorSublayerCycle.txt
            --diff-compare=compositionErrors_ErrorSublayerCycle.txt
    )

    add_test(NAME testPcpMuseum_ErrorOwner
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorOwner.txt --session ErrorOwner/session.usda ErrorOwner/root.usda"
            --testenv-dir=testPcpMuseum_ErrorOwner.testenv
            --diff-stdout=compositionResults_ErrorOwner.txt
            --diff-compare=compositionErrors_ErrorOwner.txt
    )

    add_test(NAME testPcpMuseum_ErrorInvalidAuthoredRelocates
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInvalidAuthoredRelocates.txt ErrorInvalidAuthoredRelocates/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInvalidAuthoredRelocates.testenv
            --diff-stdout=compositionResults_ErrorInvalidAuthoredRelocates.txt
            --diff-compare=compositionErrors_ErrorInvalidAuthoredRelocates.txt
    )

    # XXX:
    # Temporarily disable this test on Windows; the compositionErrors output
    # contains extra new lines that don't show up for any other build.
    if (NOT WIN32)
        add_test(NAME testPcpMuseum_ErrorInvalidConflictingRelocates
            COMMAND ${testWrapper}
                "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInvalidConflictingRelocates.txt ErrorInvalidConflictingRelocates/root.usda"
                --testenv-dir=testPcpMuseum_ErrorInvalidConflictingRelocates.testenv
                --diff-stdout=compositionResults_ErrorInvalidConflictingRelocates.txt
                --diff-compare=compositionErrors_ErrorInvalidConflictingRelocates.txt
                --env-var=PCP_ENABLE_LEGACY_RELOCATES_BEHAVIOR=0
        )
    endif()

    add_test(NAME testPcpMuseum_ErrorInvalidReferenceToRelocationSource
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorInvalidReferenceToRelocationSource.txt ErrorInvalidReferenceToRelocationSource/root.usda"
            --testenv-dir=testPcpMuseum_ErrorInvalidReferenceToRelocationSource.testenv
            --diff-stdout=compositionResults_ErrorInvalidReferenceToRelocationSource.txt
            --diff-compare=compositionErrors_ErrorInvalidReferenceToRelocationSource.txt
    )

    add_test(NAME testPcpMuseum_ErrorRelocateWithVariantSelection
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_ErrorRelocateWithVariantSelection.txt ErrorRelocateWithVariantSelection/root.usda"
            --testenv-dir=testPcpMuseum_ErrorRelocateWithVariantSelection.testenv
            --diff-stdout=compositionResults_ErrorRelocateWithVariantSelection.txt
            --diff-compare=compositionErrors_ErrorRelocateWithVariantSelection.txt
            --env-var=SDF_SCHEMA_PROHIBIT_INVALID_VARIANT_SELECTIONS=0
    )

    add_test(NAME testPcpMuseum_VariantSelectionsInVariantsOrder
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd VariantSelectionsInVariantsOrder/root.usda"
            --testenv-dir=testPcpMuseum_VariantSelectionsInVariantsOrder.testenv
            --diff-stdout=compositionResults_VariantSelectionsInVariantsOrder.txt
    )

    add_test(NAME testPcpMuseum_VariantSpecializesAndReferenceSurprisingBehavior
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd VariantSpecializesAndReferenceSurprisingBehavior/root.usda"
            --testenv-dir=testPcpMuseum_VariantSpecializesAndReferenceSurprisingBehavior.testenv
            --diff-stdout=compositionResults_VariantSpecializesAndReferenceSurprisingBehavior.txt
    )

    add_test(NAME testPcpMuseum_VariantSpecializesAndReference
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd VariantSpecializesAndReference/root.usda"
            --testenv-dir=testPcpMuseum_VariantSpecializesAndReference.testenv
            --diff-stdout=compositionResults_VariantSpecializesAndReference.txt
    )

    add_test(NAME testPcpCache
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCache.py"
            --testenv-dir=testPcpMuseum_BasicReference.testenv
    )

    add_test(NAME testPcpChanges
        COMMAND ${testWrapper}
            "${_testDir}/testPcpChanges.py"
            --testenv-dir=testPcpChanges.testenv
    )

    add_test(NAME testPcpDependencies
        COMMAND ${testWrapper}
            "${_testDir}/testPcpDependencies.py"
            --testenv-dir=testPcpDependencies.testenv
    )

    add_test(NAME testPcpInstanceKey
        COMMAND ${testWrapper}
            "${_testDir}/testPcpInstanceKey.py"
            --testenv-dir=testPcpInstanceKey.testenv
    )

    add_test(NAME testPcpLayerMuting
        COMMAND ${testWrapper}
            "${_testDir}/testPcpLayerMuting.py"
            --testenv-dir=testPcpLayerMuting.testenv
    )

    add_test(NAME testPcpRegressionBugs_bug69932
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd bug69932/root.usda"
            --testenv-dir=testPcpRegressionBugs_bug69932.testenv
            --diff-stdout=compositionResults_bug69932.txt
    )

    add_test(NAME testPcpRegressionBugs_bug70951
        COMMAND ${testWrapper}
            "${_testDir}/testPcpRegressionBugs_bug70951.py"
            --testenv-dir=testPcpRegressionBugs_bug70951.testenv
    )

    add_test(NAME testPcpRegressionBugs_bug74847
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd bug74847/root.usda"
            --testenv-dir=testPcpRegressionBugs_bug74847.testenv
            --diff-stdout=compositionResults_bug74847.txt
    )

    add_test(NAME testPcpRegressionBugs_bug82180
        COMMAND ${testWrapper}
            "${_testDir}/testPcpRegressionBugs_bug82180.py"
            --testenv-dir=testPcpRegressionBugs_bug82180.testenv
    )

    add_test(NAME testPcpRegressionBugs_bug90508
        COMMAND ${testWrapper}
            "${_testDir}/testPcpRegressionBugs_bug90508.py"
            --testenv-dir=testPcpRegressionBugs_bug90508.testenv
    )

    add_test(NAME testPcpRegressionBugs_bug90706
        COMMAND ${testWrapper}
            "${_testDir}/testPcpRegressionBugs_bug90706.py"
            --testenv-dir=testPcpRegressionBugs_bug90706.testenv
    )

    add_test(NAME testPcpRegressionBugs_bug92955
        COMMAND ${testWrapper}
            "${_testDir}/testPcpRegressionBugs_bug92955.py"
            --testenv-dir=testPcpRegressionBugs_bug92955.testenv
    )

    add_test(NAME testPcpRegressionBugs_bug92827
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors_bug92827.txt bug92827/root.usda"
            --testenv-dir=testPcpRegressionBugs_bug92827.testenv
            --diff-stdout=compositionResults_bug92827.txt
            --diff-compare=compositionErrors_bug92827.txt
    )

    add_test(NAME testPcpRegressionBugs_bug101300
        COMMAND ${testWrapper}
            "${_testDir}/testPcpRegressionBugs_bug101300.py"
            --testenv-dir=testPcpRegressionBugs_bug101300.testenv
    )

    add_test(NAME testPcpRegressionBugs_USD-6755
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd --errorFile compositionErrors.txt root.usda"
            --testenv-dir=testPcpRegressionBugs_USD-6755.testenv
            --diff-stdout=compositionResults.txt
            --diff-compare=compositionErrors.txt
    )

    add_test(NAME testPcpRegressionBugs_case1
        COMMAND ${testWrapper}
            "${_testDir}/testPcpCompositionResults.py --usd case1/root.usda"
            --testenv-dir=testPcpRegressionBugs_case1.testenv
            --diff-stdout=compositionResults_case1.txt
    )

    add_test(NAME testPcpResolvedPathChange
        COMMAND ${testWrapper}
            "${_testDir}/testPcpResolvedPathChange.py"
    )

    add_test(NAME testPcpMapFunction
        COMMAND ${testWrapper}
            "${_testDir}/testPcpMapFunction.py"
    )

    add_test(NAME testPcpPathTranslation
        COMMAND ${testWrapper}
            "${_testDir}/testPcpPathTranslation.py"
            --testenv-dir=testPcpPathTranslation.testenv
    )

    add_test(NAME testPcpPrimIndex
        COMMAND ${testWrapper}
            "${_testDir}/testPcpPrimIndex.py"
    )

    add_test(NAME testPcpDynamicFileFormatPlugin
        COMMAND ${testWrapper}
            "${_testDir}/testPcpDynamicFileFormatPlugin.py"
            --testenv-dir=testPcpDynamicFileFormatPlugin.testenv
            --env-var=TEST_PCP_DYNAMIC_FILE_FORMAT_TOKENS_USE_ATTRIBUTE_INPUTS=0
            "--env-var=PCP_DYNAMIC_FILE_FORMAT_PLUGIN=$<TARGET_FILE_DIR:TestPcpDynamicFileFormatPlugin>/plugInfo_$<CONFIG>.json"
            "--env-var=PCP_ASSET_PATH_DYNAMIC_FILE_FORMAT_PLUGIN=$<TARGET_FILE_DIR:TestPcpAssetPathDynamicFileFormatPlugin>/plugInfo_$<CONFIG>.json"
    )

    add_test(NAME testPcpDynamicFileFormatPlugin_AttributeInputs
        COMMAND ${testWrapper}
            "${_testDir}/testPcpDynamicFileFormatPlugin.py"
            --testenv-dir=testPcpDynamicFileFormatPlugin.testenv
            --env-var=TEST_PCP_DYNAMIC_FILE_FORMAT_TOKENS_USE_ATTRIBUTE_INPUTS=1
            "--env-var=PCP_DYNAMIC_FILE_FORMAT_PLUGIN=$<TARGET_FILE_DIR:TestPcpDynamicFileFormatPlugin>/plugInfo_$<CONFIG>.json"
            "--env-var=PCP_ASSET_PATH_DYNAMIC_FILE_FORMAT_PLUGIN=$<TARGET_FILE_DIR:TestPcpAssetPathDynamicFileFormatPlugin>/plugInfo_$<CONFIG>.json"
    )

    add_test(NAME testPcpExpressionComposition
        COMMAND ${testWrapper}
            "${_testDir}/testPcpExpressionComposition.py"
            --testenv-dir=testPcpExpressionComposition.testenv
    )

    add_test(NAME testPcpExpressionVariables
        COMMAND ${testWrapper}
            "${_testDir}/testPcpExpressionVariables.py"
    )

    add_test(NAME testPcpExpressionVariablesSource
        COMMAND ${testWrapper}
            "${_testDir}/testPcpExpressionVariablesSource.py"
    )

    add_test(NAME testPcpStreamingLayerReload
        COMMAND ${testWrapper}
            "${_testDir}/testPcpStreamingLayerReload.py"
            --testenv-dir=testPcpStreamingLayerReload.testenv
            "--env-var=PCP_STREAMING_LAYER_RELOAD_PLUGIN=$<TARGET_FILE_DIR:TestPcpStreamingLayerReload>/plugInfo_$<CONFIG>.json"
    )

    add_test(NAME testPcpOwner
        COMMAND ${testWrapper}
            "${_testDir}/testPcpOwner.py"
            --testenv-dir=testPcpOwner.testenv
    )

    add_test(NAME testPcpLayerRelocatesEditBuilder
        COMMAND ${testWrapper}
            "${_testDir}/testPcpLayerRelocatesEditBuilder.py"
    )

    add_test(NAME testPcpNegativeOffsetScale
        COMMAND ${testWrapper}
            "${_testDir}/testPcpNegativeOffsetScale.py"
    )
endif()
