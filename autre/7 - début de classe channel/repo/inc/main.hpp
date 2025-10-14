#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <list>
#include <map>
#include <vector>
#include <set>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <csignal>

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

extern volatile sig_atomic_t g_stop_requested;

// signal.cpp
void set_signal_handlers();

// utils.cpp
void perror_and_throw(const char* message);
void error_and_throw(const char* message);
