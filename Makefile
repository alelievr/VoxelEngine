# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: alelievr <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2014/07/15 15:13:38 by alelievr          #+#    #+#              #
#    Updated: 2019/03/24 19:54:32 by alelievr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#################
##  VARIABLES  ##
#################

#	Sources
SRCDIR		=	Sources
SRC			=	Main.cpp			\

#	Objects
OBJDIR		=	obj

#	Variables
LIBFT		=	2	#1 or 0 to include the libft / 2 for autodetct
DEBUGLEVEL	=	0	#can be 0 for no debug 1 for or 2 for harder debug
					#Warrning: non null debuglevel will disable optlevel
OPTLEVEL	=	1	#same than debuglevel
					#Warrning: non null optlevel will disable debuglevel
CPPVERSION	=	c++1z
#For simpler and faster use, use commnd line variables DEBUG and OPTI:
#Example $> make DEBUG=2 will set debuglevel to 2

#	Includes
LWGC_PATH	=	Deps/LWGC
INCDIRS		=	Sources $(LWGC_PATH)/Sources ${VULKAN_SDK}/include

#	Libraries
LWGC_DEPS	=	$(LWGC_PATH)/Deps
LIBDIRS		=	$(LWGC_PATH) $(LWGC_DEPS) $(LWGC_DEPS)/glfw/src/ $(LWGC_DEPS)/imgui/ $(LWGC_DEPS)/glslang/build/SPIRV $(LWGC_DEPS)/glslang/build/hlsl $(LWGC_DEPS)/glslang/build/glslang ${VULKAN_SDK}/lib $(LWGC_DEPS)/SPIRV-Cross
LDLIBS		=	-lLWGC -lglfw3 -lImGUI -lvulkan -lglslang -lSPIRV -lHLSL -lSPVRemapper $(LWGC_DEPS)/SPIRV-Cross/libspirv-cross.a
LWGCLIB		=	Deps/LWGC/libLWGC.a

#	Output
NAME		=	vox

#	Compiler
WERROR		=	#-Werror
CFLAGS		=	-Wall -Wextra -pedantic -ffast-math -ffunction-sections -fdata-sections
CPPFLAGS	=	-Wno-c++98-compat
CPROTECTION	=	-z execstack -fno-stack-protector

DEBUGFLAGS1	=	-ggdb -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -O0
DEBUGFLAGS2	=	-fsanitize-memory-track-origins=2
OPTFLAGS1	=	-funroll-loops -O2
OPTFLAGS2	=	-pipe -funroll-loops -Ofast

#################
##  COLORS     ##
#################
CPREFIX		=	"\033[38;5;"
BGPREFIX	=	"\033[48;5;"
CCLEAR		=	"\033[0m"
CLINK_T		=	$(CPREFIX)"129m"
CLINK		=	$(CPREFIX)"93m"
COBJ_T		=	$(CPREFIX)"119m"
COBJ		=	$(CPREFIX)"113m"
CCLEAN_T	=	$(CPREFIX)"9m"
CCLEAN		=	$(CPREFIX)"166m"
CRUN_T		=	$(CPREFIX)"198m"
CRUN		=	$(CPREFIX)"163m"
CDEPEND		=	$(CPREFIX)"231m"
CDEPEND_T	=	$(CPREFIX)"231m"
CNORM_T		=	"226m"
CNORM_ERR	=	"196m"
CNORM_WARN	=	"202m"
CNORM_OK	=	"231m"

#################
##  OS/PROC    ##
#################

OS			:=	$(shell uname -s)
PROC		:=	$(shell uname -p)
DEBUGFLAGS	=	
LINKDEBUG	=	
OPTFLAGS	=	
#COMPILATION	=	

ifeq "$(OS)" "Windows_NT"
endif
ifeq "$(OS)" "Linux"
	LDLIBS		+= 
	DEBUGFLAGS	+= -fsanitize=memory -fsanitize-memory-use-after-dtor -fsanitize=thread
endif
ifeq "$(OS)" "Darwin"
	FRAMEWORK	=	
endif

COMPILER	=	$(shell readlink $(which cc))
ifneq (,$(findstring clang++,$(COMPILER)))
	#Clang++ compiler
	CFLAGS	+= -dead_strip
else ifneq (,$(findstring clang,$(COMPILER)))
	#Clang compiler
	CFLAGS	+= -dead_strip
else ifneq (,$(findstring gcc,$(COMPILER)))
	#GCC compiler
	CFLAGS	+=	--gc-sections
else ifneq (,$(findstring gcc++,$(COMPILER)))
	#G++ compiler
	CFLAGS	+=	--gc-sections
endif

#################
##  AUTO       ##
#################

NASM		=	nasm
OBJS		=	$(patsubst %.c,%.o, $(filter %.c, $(SRC))) \
				$(patsubst %.cpp,%.o, $(filter %.cpp, $(SRC))) \
				$(patsubst %.s,%.o, $(filter %.s, $(SRC)))
OBJ			=	$(addprefix $(OBJDIR)/,$(notdir $(OBJS)))
NORME		=	**/*.[ch]
VPATH		+=	$(dir $(addprefix $(SRCDIR)/,$(SRC)))
VFRAME		=	$(addprefix -framework ,$(FRAMEWORK))
INCFILES	=	$(foreach inc, $(INCDIRS), $(wildcard $(inc)/*.h))
INCFLAGS	=	$(addprefix -I,$(INCDIRS))
LDFLAGS		=	$(addprefix -L,$(LIBDIRS))
LINKER		=	$(CC)

disp_indent	=	tabs=""; \
				for I in `seq 1 $(MAKELEVEL)`; do \
					test "$(MAKELEVEL)" '!=' '0' && tabs=$$tabs"\t"; \
				done

color_exec	=	$(call disp_indent); \
				echo $$tabs$(1)➤ $(3)$(2); \
				echo $$tabs '$(strip $(4))' $(CCLEAR); \
				$(4)

color_exec_t=	$(call disp_indent); \
				echo $(1)➤ '$(strip $(3))'$(2);$(3);printf $(CCLEAR)

ifneq ($(filter 1,$(strip $(DEBUGLEVEL)) ${DEBUG}),)
	OPTLEVEL = 0
	OPTI = 0
	DEBUGFLAGS += $(DEBUGFLAGS1)
endif
ifneq ($(filter 2,$(strip $(DEBUGLEVEL)) ${DEBUG}),)
	OPTLEVEL = 0
	OPTI = 0
	DEBUGFLAGS += $(DEBUGFLAGS1)
	LINKDEBUG += $(DEBUGFLAGS1) $(DEBUGFLAGS2)
	export ASAN_OPTIONS=check_initialization_order=1
endif

ifneq ($(filter 1,$(strip $(OPTLEVEL)) ${OPTI}),)
	DEBUGFLAGS = 
	OPTFLAGS = $(OPTFLAGS1)
endif
ifneq ($(filter 2,$(strip $(OPTLEVEL)) ${OPTI}),)
	DEBUGFLAGS = 
	OPTFLAGS = $(OPTFLAGS1) $(OPTFLAGS2)
endif

ifndef $(CXX)
	CXX = clang++
endif

ifneq ($(filter %.cpp,$(SRC)),)
	LINKER = $(CXX)
endif

ifdef ${NOWERROR}
	WERROR = 
endif

ifeq "$(strip $(LIBFT))" "2"
ifneq ($(wildcard ./libft),)
	LIBDIRS += "libft"
	LDLIBS += "-lft"
	INCDIRS += "libft/include"
endif
endif

#################
##  TARGETS    ##
#################

#	First target
all: $(NAME)

$(LWGCLIB):
	make -C Deps/LWGC

#	Linking
$(NAME): $(LWGCLIB) $(OBJ)
	@$(if $(findstring lft,$(LDLIBS)),$(call color_exec_t,$(CCLEAR),$(CCLEAR),\
		make -j 4 -C libft))
	@$(call color_exec,$(CLINK_T),$(CLINK),"Link of $(NAME):",\
		$(LINKER) $(WERROR) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(OPTFLAGS) $(DEBUGFLAGS) $(LINKDEBUG) $(VFRAME) -o $@ $^)

$(OBJDIR)/%.o: %.cpp $(INCFILES)
	@mkdir -p $(OBJDIR)/$(dir $<)
	@$(call color_exec,$(COBJ_T),$(COBJ),"Object: $@",\
		$(CXX) -std=$(CPPVERSION) $(WERROR) $(CFLAGS) $(OPTFLAGS) $(DEBUGFLAGS) $(CPPFLAGS) $(INCFLAGS) -o $@ -c $<)

#	Objects compilation
$(OBJDIR)/%.o: %.c $(INCFILES)
	@mkdir -p $(OBJDIR)/$(dir $<)
	@$(call color_exec,$(COBJ_T),$(COBJ),"Object: $@",\
		$(CC) $(WERROR) $(CFLAGS) $(OPTFLAGS) $(DEBUGFLAGS) $(INCFLAGS) -o $@ -c $<)

$(OBJDIR)/%.o: %.s
	@mkdir -p $(OBJDIR)/$(dir $<)
	@$(call color_exec,$(COBJ_T),$(COBJ),"Object: $@",\
		$(NASM) -f macho64 -o $@ $<)

#	Removing objects
clean:
	@$(call color_exec,$(CCLEAN_T),$(CCLEAN),"Clean:",\
		$(RM) $(OBJ))
	@rm -rf $(OBJDIR)

#	Removing objects and exe
fclean: clean
	@$(call color_exec,$(CCLEAN_T),$(CCLEAN),"Fclean:",\
		$(RM) $(NAME))

#	All removing then compiling
re: fclean all

f:	all run

#	Checking norme
norme:
	@norminette $(NORME) | sed "s/Norme/[38;5;$(CNORM_T)➤ [38;5;$(CNORM_OK)Norme/g;s/Warning/[0;$(CNORM_WARN)Warning/g;s/Error/[0;$(CNORM_ERR)Error/g"

run: $(NAME)
	@echo $(CRUN_T)"➤ "$(CRUN)"./$(NAME) ${ARGS}\033[0m"
	@./$(NAME) ${ARGS}

codesize:
	@cat $(NORME) |grep -v '/\*' |wc -l

functions: $(NAME)
	@nm $(NAME) | grep U

coffee:
	@clear
	@echo ""
	@echo "                   ("
	@echo "	                     )     ("
	@echo "               ___...(-------)-....___"
	@echo '           .-""       )    (          ""-.'
	@echo "      .-''''|-._             )         _.-|"
	@echo '     /  .--.|   `""---...........---""`   |'
	@echo "    /  /    |                             |"
	@echo "    |  |    |                             |"
	@echo "     \  \   |                             |"
	@echo "      '\ '\ |                             |"
	@echo "        '\ '|                             |"
	@echo "        _/ /\                             /"
	@echo "       (__/  \                           /"
	@echo '    _..---""` \                         /`""---.._'
	@echo " .-'           \                       /          '-."
	@echo ":               '-.__             __.-'              :"
	@echo ':                  ) ""---...---"" (                :'
	@echo "\'._                '"--...___...--"'              _.'"
	@echo '   \""--..__                              __..--""/'
	@echo "     '._     """----.....______.....----"""         _.'"
	@echo '         ""--..,,_____            _____,,..--"""'''
	@echo '                      """------"""'
	@sleep 0.5
	@clear
	@echo ""
	@echo "                 ("
	@echo "	                  )      ("
	@echo "               ___..(.------)--....___"
	@echo '           .-""       )   (           ""-.'
	@echo "      .-''''|-._      (       )        _.-|"
	@echo '     /  .--.|   `""---...........---""`   |'
	@echo "    /  /    |                             |"
	@echo "    |  |    |                             |"
	@echo "     \  \   |                             |"
	@echo "      '\ '\ |                             |"
	@echo "        '\ '|                             |"
	@echo "        _/ /\                             /"
	@echo "       (__/  \                           /"
	@echo '    _..---""` \                         /`""---.._'
	@echo " .-'           \                       /          '-."
	@echo ":               '-.__             __.-'              :"
	@echo ':                  ) ""---...---"" (                :'
	@echo "\'._                '"--...___...--"'              _.'"
	@echo '   \""--..__                              __..--""/'
	@echo "     '._     """----.....______.....----"""         _.'"
	@echo '         ""--..,,_____            _____,,..--"""'''
	@echo '                      """------"""'
	@sleep 0.5
	@clear
	@echo ""
	@echo "               ("
	@echo "	                  )     ("
	@echo "               ___..(.------)--....___"
	@echo '           .-""      )    (           ""-.'
	@echo "      .-''''|-._      (       )        _.-|"
	@echo '     /  .--.|   `""---...........---""`   |'
	@echo "    /  /    |                             |"
	@echo "    |  |    |                             |"
	@echo "     \  \   |                             |"
	@echo "      '\ '\ |                             |"
	@echo "        '\ '|                             |"
	@echo "        _/ /\                             /"
	@echo "       (__/  \                           /"
	@echo '    _..---""` \                         /`""---.._'
	@echo " .-'           \                       /          '-."
	@echo ":               '-.__             __.-'              :"
	@echo ':                  ) ""---...---"" (                :'
	@echo "\'._                '"--...___...--"'              _.'"
	@echo '   \""--..__                              __..--""/'
	@echo "     '._     """----.....______.....----"""         _.'"
	@echo '         ""--..,,_____            _____,,..--"""'''
	@echo '                      """------"""'
	@sleep 0.5
	@clear
	@echo ""
	@echo "             (         ) "
	@echo "	              )        ("
	@echo "               ___)...----)----....___"
	@echo '           .-""      )    (           ""-.'
	@echo "      .-''''|-._      (       )        _.-|"
	@echo '     /  .--.|   `""---...........---""`   |'
	@echo "    /  /    |                             |"
	@echo "    |  |    |                             |"
	@echo "     \  \   |                             |"
	@echo "      '\ '\ |                             |"
	@echo "        '\ '|                             |"
	@echo "        _/ /\                             /"
	@echo "       (__/  \                           /"
	@echo '    _..---""` \                         /`""---.._'
	@echo " .-'           \                       /          '-."
	@echo ":               '-.__             __.-'              :"
	@echo ':                  ) ""---...---"" (                :'
	@echo "\'._                '"--...___...--"'              _.'"
	@echo '   \""--..__                              __..--""/'
	@echo "     '._     """----.....______.....----"""         _.'"
	@echo '         ""--..,,_____            _____,,..--"""'''
	@echo '                      """------"""'

.PHONY: all clean fclean re norme codesize
