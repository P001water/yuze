cmake_minimum_required(VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("yuze" C)

# 设置源代码文件
set(SRCFILE
    "src/debug.c"
    "src/forward.c"
    "src/getopt.c"
    "src/main.c"
    "src/portTransmit.c"
    "src/reverse.c"
    "src/socks5Proto.c"
    "src/socks5Proxy.c"
    "src/socksAPI.c"
    "src/tunnelSock2sock.c"
)

# 将源代码添加到此项目的可执行文件。
add_executable(yuze ${SRCFILE})
target_link_libraries(yuze pthread)

# 设置 C 标准
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET yuze PROPERTY C_STANDARD 11)
endif()

# TODO: 如有需要，请添加测试并安装目标。
