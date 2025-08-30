# gérer les arguments

```
./ircserv <port> <password>
```

port doit être un nombre

voir les limites sur le password

# fonctions à creuser

- setsockopt
- getsockname
- getprotobyname
- gethostbyname
- getaddrinfo
- freeaddrinfo
- inet_addr
- inet_ntoa
- signal
- sigaction
- lseek
- fstat
- fcntl (spécialement sur mac, pour write, pour rendre les fd non bloquants. seulement autorisé : fcntl(fd, F_SETFL, O_NONBLOCK);)
- poll / select / kqueue / epoll (1 seul appel autorisé et nécessaire)

