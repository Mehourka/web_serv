# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kmehour <kmehour@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/01/11 12:46:32 by zvan-de-          #+#    #+#              #
#    Updated: 2024/10/23 20:15:11 by kmehour          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


RT	= \033[0m
G	= \033[0;32m
B	= \033[0;34m
R 	= \033[0;31m

#------------------------------------------------------------------------------#
#                                VARIABLES                                     #
#------------------------------------------------------------------------------#

# Program name
NAME 			= webserv

# Compiler and flags
CC				= c++
CFLAGS			= -Wall -Wextra -Werror -g

# others
RM				= rm -f
MAKE			= make

# Objects
OBJS_PATH		= objs/
OBJS			= $(patsubst $(SRCS_PATH)%.cpp, $(OBJS_PATH)%.o, $(SRCS_FILES))

# Test Objects
TEST_PATH		= unit_tests/
TEST_SRC		= $(wildcard $(TEST_PATH)*.cpp)
TEST_OBJ		= $(patsubst $(TEST_PATH)%.cpp, $(TEST_PATH)%.cpp, $(TEST_SRC))
TEST_EXEC		= tests

# Sources
SRCS_PATH		= src/
SRCS			= $(addprefix $(SRCS_PATH), $(SRCS_FILES))

# Includes
INCLUDES_PATH	= includes/
HEADERS_FILES	= $(wildcard $(INCLUDES_PATH)*.hpp)
HEADERS			= $(addprefix -I, $(INCLUDES_PATH))

# library and source files
SRCS_FILES		    = $(wildcard $(SRCS_PATH)*.cpp)

# Progress bar variables
TOTAL 			= $(words $(SRCS_FILES))
CURR  			= 0
PERCENT 		= 0


define update_progress
    $(eval CURR=$(shell echo $$(($(CURR) + 1))))
    $(if $(TOTAL), \
        $(eval PERCENT=$(shell echo $$(($(CURR) * 100 / $(TOTAL))))) \
    )
    @printf "\r\\033[K$(B)$(NAME): $(RT) $(PERCENT)%% ["
    @for i in `seq 1 $(PERCENT)`; do \
        printf "$(G)=$(RT)"; \
    done
	@for i in `seq $(PERCENT) 100`; do \
        printf " "; \
    done
    @printf "]"
endef

define print_header
    @echo "$$HEADER"
endef


#------------------------------------------------------------------------------#
#                                 RULES                                        #
#------------------------------------------------------------------------------#

all: $(HEAD) $(NAME)

$(NAME): $(OBJS_PATH) $(OBJS)
	@$(CC)  $(CFLAGS) -o $@ $(OBJS) $(HEADERS)
	@echo "$(G)\n -- $(NAME) made 👾 --$(RT)"

test: $(OBJS_PATH) $(filter-out $(OBJS_PATH)main.o, $(OBJS)) $(TEST_OBJ)
	@$(CC)  $(CFLAGS) $(HEADERS) -I$(TEST_PATH) -o $(TEST_EXEC) $(filter-out $(OBJS_PATH)main.o, $(OBJS)) $(TEST_OBJ)
	@./$(TEST_EXEC)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp $(HEADERS_FILES)
	@$(CC) $(CFLAGS) -o $@ -c $<
	$(call update_progress)

$(OBJS_PATH):
	@mkdir -p $(OBJS_PATH)

run: all
	@./$(NAME) config/default.conf

errors: all
	@for i in {0..13}; do \
		./$(NAME) config/errors/error$$i.conf; \
	done

clean:
	@rm -rf $(OBJS) $(OBJS_PATH) $(TEST_EXEC).dSYM/
	@echo "$(R)Files succesfully cleaned 🗑$(RT)"

fclean: clean
	@$(RM) $(NAME) $(NAME_EXPLAIN) $(TEST_EXEC)

re: fclean all

tester: all
	./tester http://localhost:8001

.PHONY:		all clean fclean re explain tester test

#------------------------------------------------------------------------------#
#                                 VALGRIND                                     #
#------------------------------------------------------------------------------#

vleaks:
	valgrind \
	--leak-check=full \
	--show-leak-kinds=all \
	--show-reachable=yes \
	--show-possibly-lost=yes \
	--track-origins=yes \
	./$(NAME) config/default.conf

vleaks_fd:
	valgrind \
	--leak-check=summary \
	--show-leak-kinds=all \
	--show-reachable=yes \
	--show-possibly-lost=yes \
	--track-origins=yes \
	--track-fds=all \
	./$(NAME) config/default.conf
