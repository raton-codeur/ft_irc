NAME = ircserv
FLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc -g3 -fsanitize=address
SRCS = $(addprefix src/, \
	main.cpp \
	Server.cpp \
	Client.cpp \
	CommandHandler.cpp \
	Channel.cpp \
	utils.cpp \
	signals.cpp \
)
OBJS = $(patsubst src/%.cpp, tmp/%.o, $(SRCS))

tmp/%.o : src/%.cpp
	@ mkdir -p tmp
	c++ $(FLAGS) -c $< -o $@

$(NAME) : $(OBJS)
	c++ $(FLAGS) $(OBJS) -o $@

all : $(NAME)

clean :
	reset
	@ rm -rf tmp && echo "clean"

fclean : clean
	@ rm -f $(NAME) && echo "fclean"

re : fclean all

run : all
	./$(NAME) 6667 okok

nc :
	nc localhost 6667

irssi :
	irssi --config irssi.config

.PHONY: all clean fclean re run nc irssi
