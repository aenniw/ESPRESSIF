#!/bin/bash

for PROJECT in ./*/platformio.ini; do
    PROJECT_DIR=${PROJECT%/platformio.ini}

    platformio run --project-dir ${PROJECT_DIR} || \
        { echo "Project: ${PROJECT} compilation failed"; exit 1; }

    platformio check --skip-packages --fail-on-defect medium --project-dir ${PROJECT_DIR} || \
        { echo "Project: ${PROJECT} checks failed"; exit 2; }

    if [[ -d ${PROJECT_DIR}/resources ]]; then
        platformio run -t "buildfs" --project-dir ${PROJECT_DIR} || \
            { echo "Project: ${PROJECT} ffs failed"; exit 3; }
    fi
done