NAME = ircserv
FLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc -g3
SRCS = $(addprefix src/, \
	main.cpp \
	Server.cpp \
	Client.cpp \
	Channel.cpp \
	CommandHandler.cpp \
	utils.cpp \
	signals.cpp \
)
OBJS = $(patsubst src/%.cpp, tmp/%.o, $(SRCS))

all : $(NAME)

tmp/%.o : src/%.cpp
	@ mkdir -p tmp
	c++ $(FLAGS) -c $< -o $@

$(NAME) : $(OBJS)
	c++ $(FLAGS) $(OBJS) -o $@

clean :
	reset
	@ rm -rf tmp && echo "clean"

fclean : clean
	@ rm -f $(NAME) && echo "fclean"

re : fclean all

test : re
	./$(NAME) 6667 okok

nc :
	nc 127.0.0.1 6667

irssi :
	irssi --config irssi.config

.PHONY: all clean fclean re test nc irssi
