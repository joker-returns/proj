CC = g++
CFLAGS = -g -Wall -Wno-write-strings -Wno-sign-compare
SRCS = main.cpp
PROG = seam_carving

OPENCV = `pkg-config opencv --cflags --libs`

LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS) 