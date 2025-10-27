#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <deque>
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

extern volatile sig_atomic_t g_stopRequested;

// signals.cpp
void setSignalHandlers();
void checkSignals();

// utils.cpp
void perrorAndThrow(const char* message);
void errorAndThrow(const char* message);
