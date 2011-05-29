C = cc
CFLAGS = -pedantic -Wall -Wextra -O4

chktcpport : chktcpport.o
	$(C) -o chktcpport chktcpport.o

.PHONY : clean all

clean :
	-rm chktcpport chktcpport.o

all : chktcpport

