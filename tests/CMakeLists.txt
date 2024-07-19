set(SOURCE_FILES
    ../src/getdelim.c
    ../src/getline.c
    # cbrutekrag.c
    ../src/log.c
    ../src/str.c
    # iprange.c
    # progressbar.c
    # bruteforce_ssh.c
    # detection.c
    # target.c
    # credentials.c
    # macrowrapper.c
  )

# Test credentials parse
add_executable(test1 test-credentials.c ${SOURCE_FILES} ../src/credentials.c)

# Test target parse
add_executable(test2 test-targets.c ${SOURCE_FILES} ../src/iprange.c ../src/target.c)
if(MINGW)
  target_link_libraries(test2 PRIVATE ws2_32)
endif()

# Añade un test
add_test(NAME Credentials COMMAND test1)
add_test(NAME TargetParse COMMAND test2)
