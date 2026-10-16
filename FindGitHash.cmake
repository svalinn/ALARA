execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

file(WRITE "${OUTPUT_FILE}"
    "#pragma once\n"
    "#define GIT_COMMIT_HASH \"${GIT_COMMIT_HASH}\"\n"
)