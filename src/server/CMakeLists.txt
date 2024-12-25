aux_source_directory(. SRC_LIST)
aux_source_directory(../database/. DB_LIST)
aux_source_directory(../moudle/. MODULE_LIST)
aux_source_directory(../../net/. NET_LIST)

add_executable(ChatServer ${NET_LIST} ${SRC_LIST} ${DB_LIST} ${MODULE_LIST})

target_link_libraries(ChatServer mysqlclient hiredis pthread)