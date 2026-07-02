# Try to locate the Pico SDK in the standard locations.
if (DEFINED ENV{PICO_SDK_PATH} AND EXISTS "$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")
elseif (EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico-sdk/external/pico_sdk_import.cmake")
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
    include("${CMAKE_CURRENT_LIST_DIR}/pico-sdk/external/pico_sdk_import.cmake")
elseif (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk/external/pico_sdk_import.cmake")
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk")
    include("${CMAKE_CURRENT_LIST_DIR}/../pico-sdk/external/pico_sdk_import.cmake")
else()
    message(FATAL_ERROR
        "Pico SDK not found. Install it or set PICO_SDK_PATH to the SDK root."
    )
endif()
