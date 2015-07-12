#!/bin/bash

CARLA_SRC="/home/falktx/Personal/FOSS/GIT/falkTX/Carla/source"

cp "$CARLA_SRC/includes/CarlaDefines.h" .
cp "$CARLA_SRC/includes/CarlaMIDI.h" .

cp "$CARLA_SRC/utils/CarlaExternalUI.hpp" .
cp "$CARLA_SRC/utils/CarlaMutex.hpp" .
cp "$CARLA_SRC/utils/CarlaString.hpp" .
cp "$CARLA_SRC/utils/CarlaThread.hpp" .
cp "$CARLA_SRC/utils/CarlaJuceUtils.hpp" .
cp "$CARLA_SRC/utils/CarlaMathUtils.hpp" .
cp "$CARLA_SRC/utils/CarlaPipeUtils.cpp" .
cp "$CARLA_SRC/utils/CarlaPipeUtils.hpp" .
cp "$CARLA_SRC/utils/CarlaUtils.hpp" .
