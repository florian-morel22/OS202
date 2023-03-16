# Configuration pour MSYS 2
# Peut être modifié à votre convenance...
#CXX = clang++ 
CXX = mpic++
#export OMPI_CC=gcc-10
#export OMPI_CXX=g++-10
CXXFLAGS = -std=c++17 -I/usr/local/Cellar/sfml/2.5.1_2/include/
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -fsanitize=address -openmp
CXXFLAGS2 = CXXFLAGS
else
CXXFLAGS2 = ${CXXFLAGS} -O2 -march=native -Wall -openmp
CXXFLAGS += -O3 -march=native -Wall -openmp
endif
LIB=-L/usr/local/lib/ -lsfml-graphics -lsfml-window -lsfml-system


#-L/usr/local//Cellar/sfml/2.5.1_2/lib