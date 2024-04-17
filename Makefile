NAME=webserv

SOURCES=$(wildcard source/*.cpp)
HEADERS=$(wildcard source/*.hpp)
OBJECTS=$(SOURCES:source/%.cpp=build/%.o)

CXX=c++
CXXFLAGS=-g3 -Wall -Wextra -Werror -std=c++98

# This is required to pass the evaluation
# Uncomment to build a server that doesn't turn your computer into turbo jet :^)
# Intended setup for evaluation: https://imgix.ranker.com/user_node_img/50111/1002206890/original/1002206890-photo-u1
CXXFLAGS += -D__42_LIKES_WASTING_CPU_CYCLES__

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJECTS)

build/%.o: source/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	find build -type f -name '*.o' -delete

fclean:
	find build -type f -name '*.o' -delete
	rm -f $(NAME)

re:
	make fclean
	make all

.PHONY: all clean fclean re
