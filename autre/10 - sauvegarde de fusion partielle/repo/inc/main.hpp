#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <sstream>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <csignal>

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

struct StopRequested {};

extern volatile sig_atomic_t g_stop_requested;

// signals.cpp
void set_signal_handlers();
void checkSignals();

// utils.cpp
void perror_and_throw(const char* message);
void error_and_throw(const char* message);
