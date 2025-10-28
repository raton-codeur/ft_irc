#include "main.hpp"

volatile sig_atomic_t g_stopRequested = 0;

static void sigint_handler(int signum)
{
	(void)signum;
	g_stopRequested = 1;
}

void setSignalHandlers()
{
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT,  &sa, NULL) == -1)
		perrorAndThrow("sigaction: SIGINT");
	if (sigaction(SIGTERM, &sa, NULL) == -1)
		perrorAndThrow("sigaction: SIGTERM");
	if (sigaction(SIGQUIT, &sa, NULL) == -1)
		perrorAndThrow("sigaction: SIGQUIT");
	if (sigaction(SIGHUP,  &sa, NULL) == -1)
		perrorAndThrow("sigaction: SIGHUP");

	struct sigaction sa_ign;
	sa_ign.sa_handler = SIG_IGN;
	sigemptyset(&sa_ign.sa_mask);
	sa_ign.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa_ign, NULL) == -1)
		perrorAndThrow("sigaction: SIGPIPE");
}

void checkSignals()
{
	if (g_stopRequested)
		throw StopRequested();
}
