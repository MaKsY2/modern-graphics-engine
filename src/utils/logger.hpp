#ifndef LOGGER_HPP
#define LOGGER HPP

#include <iostream>

#define LOG(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl

#endif