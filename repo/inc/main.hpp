#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <csignal>
#include <cstdio>
#include <cerrno>
#include "Data.hpp"

extern volatile sig_atomic_t g_stop_requested;

// signal.cpp
void set_handler();

// utils.cpp
void perror_and_throw(const char* message);


