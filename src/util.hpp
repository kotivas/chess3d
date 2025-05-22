#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Util {

//#define T_RESET   "\033[0m"
//#define T_BLACK   "\033[30m"      /* Black */
//#define T_RED     "\033[31m"      /* Red */
//#define T_GREEN   "\033[32m"      /* Green */
//#define T_YELLOW  "\033[33m"      /* Yellow */
//#define T_BLUE    "\033[34m"      /* Blue */
//#define T_MAGENTA "\033[35m"      /* Magenta */
//#define T_CYAN    "\033[36m"      /* Cyan */
//#define T_WHITE   "\033[37m"      /* White */
//#define T_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
//#define T_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
//#define T_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
//#define T_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
//#define T_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
//#define T_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
//#define T_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
//#define T_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define OUT_ERROR "\033[1m\033[31m[ERROR] \033[0m"
#define OUT_DEBUG "\033[1m\033[93m[DEBUG] \033[0m"
#define OUT_WARNING "\033[1m\033[33m[WARNING] \033[0m"
#define OUT_INFO "\033[1m\033[34m[INFO] \033[0m"

const std::string readFromFile(const std::string& path);

}