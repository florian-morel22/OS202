# include à modifier selon sa configuration
#include make_msys.inc 
include make_linux.inc


ALL= vortexSimulation.exe


default:	help
all: $(ALL)

clean:
	@rm -fr objs/*.o *.exe src/*~ *.png

OBJS= objs/vortex.o objs/screen.o objs/runge_kutta.o objs/cloud_of_points.o objs/cartesian_grid_of_speed.o \
      objs/vortexSimulation.o

objs/vortex.o:	src/point.hpp src/vector.hpp src/vortex.hpp src/vortex.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortex.cpp

objs/cartesian_grid_of_speed.o: src/point.hpp src/vector.hpp src/vortex.hpp src/cartesian_grid_of_speed.hpp src/cartesian_grid_of_speed.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/cartesian_grid_of_speed.cpp

objs/cloud_of_points.o: src/point.hpp src/rectangle.hpp src/cloud_of_points.hpp src/cloud_of_points.cpp 
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/cloud_of_points.cpp 

objs/runge_kutta.o:	src/vortex.hpp src/cloud_of_points.hpp src/cartesian_grid_of_speed.hpp src/runge_kutta.hpp src/runge_kutta.cpp 
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/runge_kutta.cpp

objs/screen.o:	src/vortex.hpp src/cloud_of_points.hpp src/cartesian_grid_of_speed.hpp src/screen.hpp src/screen.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/screen.cpp

objs/vortexSimulation.o: src/cartesian_grid_of_speed.hpp src/vortex.hpp src/cloud_of_points.hpp src/runge_kutta.hpp src/screen.hpp src/vortexSimulation.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortexSimulation.cpp



vortexSimulation.exe: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIB)





objs/vortexSimulation_seq.o: src/cartesian_grid_of_speed.hpp src/vortex.hpp src/cloud_of_points.hpp src/runge_kutta.hpp src/screen.hpp src/vortexSimulation_seq.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ src/vortexSimulation_seq.cpp

vortexSimulation_seq.exe :  objs/vortex.o objs/screen.o objs/runge_kutta.o objs/cloud_of_points.o objs/cartesian_grid_of_speed.o objs/vortexSimulation_seq.o
	$(CXX) $(CXXFLAGS) -o $@ objs/vortex.o objs/screen.o objs/runge_kutta.o objs/cloud_of_points.o objs/cartesian_grid_of_speed.o objs/vortexSimulation_seq.o $(LIB)

help:
	@echo "Available targets : "
	@echo "    all                           : compile all executables"
	@echo "    vortexSimulation.exe          : compile simple this executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"
