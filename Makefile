# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hozhan <hozhan@student.42kocaeli.com.tr    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/19 18:29:17 by hozhan            #+#    #+#              #
#    Updated: 2026/01/19 18:29:19 by hozhan           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = mydownloader
CC          = gcc
CFLAGS      = -Wall -Wextra -Werror
LIBS        = -lcurl
SRC         = src/main.c
OBJ         = $(SRC:.c=.o)
RM          = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBS)
	@echo "Derleme tamamlandı! Kullanım: ./$(NAME) <url>"

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re