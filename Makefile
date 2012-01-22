CXX = g++
CXXFLAGS = -O2 -Wall
CFLAGS = $(CXXFLAGS) -I$(IDIR) -I$(IDIR2)
LDFLAGS = -lSDL -lGL -lGLU -lm -lpng -lSDL_ttf -lSDL_image -L ../Tirian -lTirian
OBJDIR = Obj/
PROG = tiletest2
IDIR = Include/
IDIR2 = ../Tirian/Include/
OBJ = $(OBJDIR)main.o $(OBJDIR)utils.o $(OBJDIR)texmap.o $(OBJDIR)terrain.o $(OBJDIR)bitmap.o $(OBJDIR)fakeistream.o $(OBJDIR)terrain_segment.o $(OBJDIR)mtt.o $(OBJDIR)const.o

$(PROG): $(OBJ) /home/frax/Programowanie/Tirian/libTirian.a
	$(CXX) $(OBJ) -o $(PROG) $(CFLAGS) $(LDFLAGS)

all: $(PROG)

clean:
	rm -f $(OBJDIR)*.o

$(OBJDIR)main.o:  main.cpp $(IDIR)main.hpp $(IDIR2)
	$(CXX) -c main.cpp -o $(OBJDIR)main.o $(CFLAGS)

$(OBJDIR)const.o:  const.cpp $(IDIR)const.hpp $(IDIR2)
	$(CXX) -c const.cpp -o $(OBJDIR)const.o $(CFLAGS)
	
$(OBJDIR)utils.o:  utils.cpp $(IDIR)utils.hpp
	$(CXX) -c utils.cpp -o $(OBJDIR)utils.o $(CFLAGS)
	
$(OBJDIR)texmap.o:  texmap.cpp $(IDIR)texmap.hpp
	$(CXX) -c texmap.cpp -o $(OBJDIR)texmap.o $(CFLAGS)
	
$(OBJDIR)mtt.o:  mtt.cpp $(IDIR)mtt.hpp 
	$(CXX) -c mtt.cpp -o $(OBJDIR)mtt.o $(CFLAGS)

$(OBJDIR)terrain_segment.o:  terrain_segment.cpp $(IDIR)terrain_segment.hpp $(IDIR2)geometry.hpp 
	$(CXX) -c terrain_segment.cpp -o $(OBJDIR)terrain_segment.o $(CFLAGS)
	
$(OBJDIR)terrain.o:  terrain.cpp $(IDIR)terrain.hpp $(IDIR2)geometry.hpp $(OBJDIR)texmap.o 
	$(CXX) -c terrain.cpp -o $(OBJDIR)terrain.o $(CFLAGS)
	
$(OBJDIR)bitmap.o:  bitmap.cpp $(IDIR)bitmap.hpp
	$(CXX) -c bitmap.cpp -o $(OBJDIR)bitmap.o $(CFLAGS)
	
$(OBJDIR)fakeistream.o:  fakeistream.cpp $(IDIR)fakeistream.hpp
	$(CXX) -c fakeistream.cpp -o $(OBJDIR)fakeistream.o $(CFLAGS)
