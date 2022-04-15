NAME = opengl

SRCDIR = ./src

GLEWDIR = ./glew-2.2.0/src

GRN = \e[1;32m
RED = \e[1;31m
YEL = \e[1;33m
DEL = \e[2K\r
RST = \e[0m

CC = gcc

CXX = clang++

SRCFILES = $(SRCDIR)/main.cpp

OBJS = $(SRCFILES:.cpp=.o)

INCLUDES = -I ./include

GLFWFLAGS = $(pkg-config --cflags glfw3)

GLFWLIB = $(pkg-config --static --libs glfw3)

CXXFLAGS = -Wall -Werror -Wextra -g

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(NAME) $(OBJS) -L ./lib_glfw -L ./lib_glew -lglfw3 -lGLEW -lX11 -ldl -lGL -lpthread

clean:
	@printf "$(YEL)Deleting object files...$(RST)"
	@rm $(OBJS) 2>/dev/null || true
	@printf "$(DEL)$(GRN)Deleted object files\n$(RST)"

fclean: clean
	@printf "$(YEL)Deleting program...$(RST)"
	@rm $(NAME) 2>/dev/null || true
	@printf "$(DEL)$(GRN)Deleted program\n$(RST)"

re: fclean all

.PHONY: all clean fclean re