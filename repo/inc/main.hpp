#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <list>
#include <map>

#include <cstdio>
#include <cerrno>
#include <csignal>

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

extern volatile sig_atomic_t g_stop_requested;

// signal.cpp
void set_signal_handler();

// utils.cpp
void perror_and_throw(const char* message);
