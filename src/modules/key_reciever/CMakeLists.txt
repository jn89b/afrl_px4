px4_add_module(
    MODULE modules__key_receiver
    MAIN key_receiver
    STACK_MAIN 2500
    STACK_MAX 4000
    SRCS
        key_receiver.cpp
    DEPENDS

    )



