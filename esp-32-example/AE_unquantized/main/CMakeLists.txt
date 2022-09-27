
#
# Main component of TF Micro project 'AE_unquantized'.
#

idf_component_register(
    SRCS main.cc main_functions.cc model.cpp arduino_constants.cpp arduino_main.cpp
    PRIV_REQUIRES tflite-lib spi_flash driver esp_timer
    INCLUDE_DIRS "")

    # Reduce the level of paranoia to be able to compile sources
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-maybe-uninitialized
    -Wno-missing-field-initializers
    -Wno-error=sign-compare
    -Wno-error=double-promotion
    -Wno-type-limits)
