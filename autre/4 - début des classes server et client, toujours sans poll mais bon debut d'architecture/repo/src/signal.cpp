#include "main.hpp"

volatile sig_atomic_t g_stop_requested = 0;

static void handler(int signum)
{
	(void)signum;
	g_stop_requested = 1;
}

void set_signal_handler()
{
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT,  &sa, NULL);
}
