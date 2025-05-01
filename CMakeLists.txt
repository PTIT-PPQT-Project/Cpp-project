cmake_minimum_required(VERSION 3.10)
project(user_registration)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main 
    main.cpp
    model/user.cpp
    db/user_repository.cpp
    utils/password_hasher.cpp
    controller/user_controller.cpp
    view/menu_view.cpp
)
