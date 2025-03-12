NAME := ft_ssl

SRCDIR := src
SOURCES := $(shell find $(SRCDIR) -name '*.c' -not -name '*_bonus.c')
OBJDIR := obj
OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:c=o))

# ifdef BONUS
# OBJECTS += $(OBJECTS_BONUS)
# endif

# INCLUDES := $(addprefix -I,$(sort $(dir $(shell find $(SRCDIR) -name '*.h' -not -name '*_bonus.h'))))
INCLUDES := -I$(SRCDIR)
CFLAGS := -Wall -Wextra -Werror
LFLAGS := 
ifdef NO_WARN
	CFLAGS :=
endif
CFLAGS += $(INCLUDES)
ifdef DEBUG
	SANITIZERS := address,leak
	# SANITIZERS += undefined,integer,implicit-conversion,local-bounds,float-divide-by-zero,nullability
	CFLAGS += -g -fsanitize=$(SANITIZERS) -fno-omit-frame-pointer
	LFLAGS += -g -fsanitize=$(SANITIZERS) -fno-omit-frame-pointer
endif

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

.GOAL: all
.PHONY: all
all: $(NAME)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)
	# rm -f $(TEST_NAME)

.PHONY: re
re: fclean all

.PHONY: bonus
bonus:
	$(MAKE) BONUS=1

.PHONY: re_bonus
re_bonus:
	$(MAKE) BONUS=1 re

# .PHONY: test
# test: all
# ifdef BONUS
# 	$(CC) $(if $(DEBUG), -g) -Wall -Wextra -Werror -DBONUS=1 src/main.c -lasm -L. -o $(TEST_NAME)
# else
# 	$(CC) $(if $(DEBUG), -g) -Wall -Wextra -Werror src/main.c -lasm -L. -o $(TEST_NAME)
# endif
# 	@echo
# 	@./$(TEST_NAME)
#
# .PHONY: bonus_test
# bonus_test:
# 	$(MAKE) BONUS=1 test
#
# .PHONY: re_test
# re_test: fclean test
#
# .PHONY: re_bonus_test
# re_bonus_test:
# 	$(MAKE) BONUS=1 re_test
