set(srcs "main.c")

set(requires "bluepad32")

set(requires "TFT_eSPI")

# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    list(APPEND srcs "sketch.cpp")
    list(APPEND requires "bluepad32_arduino" "arduino")
endif()

idf_component_register(SRCS "ball.cpp" "paddle.cpp" "game.cpp" "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES "${requires}")
