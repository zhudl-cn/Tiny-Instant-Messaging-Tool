aux_source_directory(. SRC_LIST)
aux_source_directory(../../net/. NET_LIST)

add_executable(ChatClient ${SRC_LIST} ${NET_LIST})

target_link_libraries(ChatClient pthread)