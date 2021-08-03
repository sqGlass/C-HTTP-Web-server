SRCS	= $(shell find . -name "*.cpp")

HEADERS	= $(shell find . -name "*.hpp")

OBJDIR	= obj/

OBJS	= ${SRCS:%.cpp=$(OBJDIR)%.o}

NAME	= webserver

CC		= clang++

RM		= rm -f

CFLAGS	= -Wall -Wextra -Werror

all:	${NAME}

$(OBJDIR)%.o: %.cpp $(HEADERS)
		@${CC} ${CFLAGS} -I. -c $< -o $(OBJDIR)${<:.cpp=.o}
		@echo "\033[0;36mObject "${<:.cpp=.o}" is compiled\033[0m"

${NAME}: ${OBJS}
		@${CC} ${CFLAGS} ${OBJS} -o ${NAME}
		@echo "\033[0;32mCompilation finished\033[0m"

clean:
		@${RM} ${OBJS}
		@echo "\033[0;31mObjects has been removed\033[0m"

fclean:	clean
		@${RM} ${NAME}
		@echo "\033[0;31mExecutable has been removed\033[0m"

re:		fclean all

.PHONY:		re all clean fclean