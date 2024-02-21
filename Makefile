NAME=webserv

SOURCES=$(wildcard source/*.cpp)
HEADERS=$(wildcard source/*.hpp)
OBJECTS=$(SOURCES:source/%.cpp=build/%.o)

CXXFLAGS=-g3 -Wall -Wextra -Werror -std=c++98

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
