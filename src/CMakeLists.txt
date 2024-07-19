set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

find_package(libssh CONFIG REQUIRED)
set(SOURCE_FILES
    getdelim.c
    getline.c
    cbrutekrag.c
    log.c
    str.c
    iprange.c
    progressbar.c
    bruteforce_ssh.c
    detection.c
    target.c
    credentials.c
    macrowrapper.c
  )
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# LIBSSH
if(MINGW)
  target_link_libraries(cbrutekrag PRIVATE ssh ws2_32)
else()
  target_link_libraries(cbrutekrag PRIVATE ssh)
endif()

# PTHREADS
if(MINGW)
  find_package(PThreads4W REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE PThreads4W::PThreads4W)
else()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()
