set(CODEGEN_ROOT_DIR ${RUNTIME_PKG_NAME}/tools/codegen)
set(MICRO_DIR ${TOP_DIR}/mindspore/lite/tools/converter/micro)

function(__install_micro_wrapper)
    file(GLOB NNACL_FILES GLOB ${NNACL_DIR}/*.h)
    install(FILES ${NNACL_FILES} DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl COMPONENT ${RUNTIME_COMPONENT_NAME})
    install(DIRECTORY ${NNACL_DIR}/base DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${NNACL_DIR}/int8 DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${NNACL_DIR}/fp32 DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(FILES ${NNACL_DIR}/fp32_grad/activation_grad.h DESTINATION
            ${CODEGEN_ROOT_DIR}/include/nnacl/fp32_grad COMPONENT ${RUNTIME_COMPONENT_NAME})
    install(FILES ${NNACL_DIR}/fp32_grad/softmax_cross_entropy_with_logits.h DESTINATION
            ${CODEGEN_ROOT_DIR}/include/nnacl/fp32_grad COMPONENT ${RUNTIME_COMPONENT_NAME})
    install(DIRECTORY ${NNACL_DIR}/kernel DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${NNACL_DIR}/infer DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${NNACL_DIR}/experimental DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${NNACL_DIR}/intrinsics DESTINATION ${CODEGEN_ROOT_DIR}/include/nnacl
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${MICRO_DIR}/coder/wrapper DESTINATION ${CODEGEN_ROOT_DIR}/include
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(TARGETS wrapper ARCHIVE DESTINATION ${CODEGEN_ROOT_DIR}/lib COMPONENT ${RUNTIME_COMPONENT_NAME})
    install(TARGETS nnacl_static ARCHIVE DESTINATION ${CODEGEN_ROOT_DIR}/lib COMPONENT ${RUNTIME_COMPONENT_NAME})
endfunction()

function(__install_micro_codegen)
    set(MICRO_CMSIS_DIR ${CMAKE_BINARY_DIR}/cmsis/CMSIS)
    install(DIRECTORY ${MICRO_CMSIS_DIR}/Core/Include DESTINATION ${CODEGEN_ROOT_DIR}/third_party/include/CMSIS/Core
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${MICRO_CMSIS_DIR}/DSP/Include DESTINATION ${CODEGEN_ROOT_DIR}/third_party/include/CMSIS/DSP
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${MICRO_CMSIS_DIR}/NN/Include DESTINATION ${CODEGEN_ROOT_DIR}/third_party/include/CMSIS/NN
            COMPONENT ${RUNTIME_COMPONENT_NAME} FILES_MATCHING PATTERN "*.h")
    install(TARGETS cmsis_nn ARCHIVE DESTINATION ${CODEGEN_ROOT_DIR}/third_party/lib
            COMPONENT ${RUNTIME_COMPONENT_NAME})
endfunction()