add_executable(converter
    converter.cpp
)

target_link_libraries(converter PRIVATE kleeBasic)

install(TARGETS converter RUNTIME DESTINATION bin)