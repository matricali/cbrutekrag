set(SOURCE_FILES
    ../src/getdelim.c
    ../src/getline.c
    ../src/log.c
    ../src/str.c
  )

# Test credentials parse
add_executable(test1 test-credentials.c ${SOURCE_FILES} ../src/credentials.c)

# Test target parse
add_executable(test2 test-targets.c ${SOURCE_FILES} ../src/iprange.c ../src/target.c)
if(MINGW)
  target_link_libraries(test2 PRIVATE ws2_32)
endif()

# Test progressbar
add_executable(test3 test-progressbar.c ../src/progressbar.c)

# Testcases
add_test(NAME Credentials COMMAND test1)
add_test(NAME TargetParse COMMAND test2)
add_test(NAME TargetProgressBar COMMAND test3)
