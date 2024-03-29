////////////////////////////////////
// Séparation affichage graphique et calcul
// Des problèmes d'ordres non reçus par le processus de calcul lorsque la
//        simulation est en pause
////////////////////////////////////

#include "cartesian_grid_of_speed.hpp"
#include "cloud_of_points.hpp"
#include "runge_kutta.hpp"
#include "screen.hpp"
#include "vortex.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <string>
#include <tuple>

auto readConfigFile(std::ifstream &input) {
  using point = Simulation::Vortices::point;

  int isMobile;
  std::size_t nbVortices;
  Numeric::CartesianGridOfSpeed cartesianGrid;
  Geometry::CloudOfPoints cloudOfPoints;
  constexpr std::size_t maxBuffer = 8192;
  char buffer[maxBuffer];
  std::string sbuffer;
  std::stringstream ibuffer;
  // Lit la première ligne de commentaire :
  input.getline(buffer, maxBuffer); // Relit un commentaire
  input.getline(buffer, maxBuffer); // Lecture de la grille cartésienne
  sbuffer = std::string(buffer, maxBuffer);
  ibuffer = std::stringstream(sbuffer);
  double xleft, ybot, h;
  std::size_t nx, ny;
  ibuffer >> xleft >> ybot >> nx >> ny >> h;
  cartesianGrid =
      Numeric::CartesianGridOfSpeed({nx, ny}, point{xleft, ybot}, h);
  input.getline(buffer, maxBuffer); // Relit un commentaire
  input.getline(buffer, maxBuffer); // Lit mode de génération des particules
  sbuffer = std::string(buffer, maxBuffer);
  ibuffer = std::stringstream(sbuffer);
  int modeGeneration;
  ibuffer >> modeGeneration;
  if (modeGeneration == 0) // Génération sur toute la grille
  {
    std::size_t nbPoints;
    ibuffer >> nbPoints;
    cloudOfPoints = Geometry::generatePointsIn(
        nbPoints, {cartesianGrid.getLeftBottomVertex(),
                   cartesianGrid.getRightTopVertex()});
  } else {
    std::size_t nbPoints;
    double xl, xr, yb, yt;
    ibuffer >> xl >> yb >> xr >> yt >> nbPoints;
    cloudOfPoints =
        Geometry::generatePointsIn(nbPoints, {point{xl, yb}, point{xr, yt}});
  }
  // Lit le nombre de vortex :
  input.getline(buffer, maxBuffer); // Relit un commentaire
  input.getline(buffer, maxBuffer); // Lit le nombre de vortex
  sbuffer = std::string(buffer, maxBuffer);
  ibuffer = std::stringstream(sbuffer);
  try {
    ibuffer >> nbVortices;
  } catch (std::ios_base::failure &err) {
    std::cout << "Error " << err.what() << " found" << std::endl;
    std::cout << "Read line : " << sbuffer << std::endl;
    throw err;
  }
  Simulation::Vortices vortices(
      nbVortices,
      {cartesianGrid.getLeftBottomVertex(), cartesianGrid.getRightTopVertex()});
  input.getline(buffer, maxBuffer); // Relit un commentaire
  for (std::size_t iVortex = 0; iVortex < nbVortices; ++iVortex) {
    input.getline(buffer, maxBuffer);
    double x, y, force;
    std::string sbuffer(buffer, maxBuffer);
    std::stringstream ibuffer(sbuffer);
    ibuffer >> x >> y >> force;
    vortices.setVortex(iVortex, point{x, y}, force);
  }
  input.getline(buffer, maxBuffer); // Relit un commentaire
  input.getline(buffer, maxBuffer); // Lit le mode de déplacement des vortex
  sbuffer = std::string(buffer, maxBuffer);
  ibuffer = std::stringstream(sbuffer);
  ibuffer >> isMobile;
  return std::make_tuple(vortices, isMobile, cartesianGrid, cloudOfPoints);
}

int main(int nargs, char *argv[]) {

  MPI_Comm commGlob;
  int nbp, rank;
  MPI_Init(&nargs, &argv);
  printf("nargs = %d\n", nargs);
  printf("argv[4] = %s\n", *argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &commGlob);
  MPI_Comm_size(commGlob, &nbp);
  MPI_Comm_rank(commGlob, &rank);

  char const *filename;
  if (nargs == 1) {
    std::cout << "Usage : vortexsimulator <nom fichier configuration>"
              << std::endl;
    return EXIT_FAILURE;
  }

  filename = argv[1];
  std::ifstream fich(filename);
  auto config = readConfigFile(fich);
  fich.close();

  std::size_t resx = 800, resy = 600;
  if (nargs > 3) {
    resx = std::stoull(argv[2]);
    resy = std::stoull(argv[3]);
  }

  auto vortices = std::get<0>(config);
  auto isMobile = std::get<1>(config);
  auto grid = std::get<2>(config);
  auto cloud = std::get<3>(config);

  std::cout << "######## Vortex simultor ########" << std::endl << std::endl;
  std::cout << "Press P for play animation " << std::endl;
  std::cout << "Press S to stop animation" << std::endl;
  std::cout << "Press right cursor to advance step by step in time"
            << std::endl;
  std::cout << "Press down cursor to halve the time step" << std::endl;
  std::cout << "Press up cursor to double the time step" << std::endl;

  grid.updateVelocityField(vortices);

  bool animate = false;
  double dt = 0.1;

  if (rank == 0) {
    Graphisme::Screen myScreen(
        {resx, resy}, {grid.getLeftBottomVertex(), grid.getRightTopVertex()});

    std::vector<double> dataVect;
    dataVect.resize(vortices.numberOfVortices() * 3 +
                    cloud.numberOfPoints() * 2);

    while (myScreen.isOpen()) {
      auto start = std::chrono::system_clock::now();
      bool advance = false;
      // on inspecte tous les évènements de la fenêtre qui ont été émis depuis
      // la précédente itération

      /* EVENT */
      sf::Event event;
      while (myScreen.pollEvent(event)) {
        // évènement "fermeture demandée"+ : on ferme la fenêtre
        if (event.type == sf::Event::Closed) {
          myScreen.close();
          bool run = false;
          MPI_Recv(&dataVect[0],
                   vortices.numberOfVortices() * 3 + cloud.numberOfPoints() * 2,
                   MPI_DOUBLE, 1, 10, commGlob, MPI_STATUS_IGNORE);
          MPI_Send(&run, 1, MPI_LOGICAL, 1, 20, commGlob);
        }

        if (event.type == sf::Event::Resized) {
          // on met à jour la vue, avec la nouvelle taille de la fenêtre
          myScreen.resize(event);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
          animate = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
          animate = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
          dt *= 2;
          MPI_Send(&dt, 1, MPI_DOUBLE, 1, 21, commGlob);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
          dt /= 2;
          MPI_Send(&dt, 1, MPI_DOUBLE, 1, 21, commGlob);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
          advance = true;
        }
      }

      /* CALCUL */

      if (animate | advance) {
        dataVect.clear();
        MPI_Recv(&dataVect[0],
                 vortices.numberOfVortices() * 3 + cloud.numberOfPoints() * 2,
                 MPI_DOUBLE, 1, 10, commGlob, MPI_STATUS_IGNORE);
        for (std::size_t i = 0; i < vortices.numberOfVortices(); ++i) {
          vortices.setVortex(
              i, Geometry::Point<double>{dataVect[3 * i], dataVect[3 * i + 1]},
              dataVect[3 * i + 2]);
        }
        grid.updateVelocityField(vortices);

        for (std::size_t i = 0; i < cloud.size(); ++i) {
          cloud[i].x = dataVect[vortices.numberOfVortices() * 3 + 2 * i];
          cloud[i].y = dataVect[vortices.numberOfVortices() * 3 + 2 * i + 1];
        }

        advance = false;
      }

      /* AFFICHAGE */
      myScreen.clear(sf::Color::Black);
      std::string strDt = std::string("Time step : ") + std::to_string(dt);
      myScreen.drawText(strDt,
                        Geometry::Point<double>{
                            50, double(myScreen.getGeometry().second - 96)});
      myScreen.displayVelocityField(grid, vortices);
      myScreen.displayParticles(grid, vortices, cloud);
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> diff = end - start;
      std::string str_fps =
          std::string("FPS : ") + std::to_string(1. / diff.count());
      myScreen.drawText(str_fps,
                        Geometry::Point<double>{
                            300, double(myScreen.getGeometry().second - 96)});
      myScreen.display();
    }
  }

  else if (rank == 1) {
    bool run = true;
    MPI_Request req;

    std::vector<double> dataVect;

    MPI_Irecv(&run, 1, MPI_LOGICAL, 0, 20, commGlob, &req);

    MPI_Irecv(&dt, 1, MPI_DOUBLE, 0, 21, commGlob, &req);

    int k = 0;
    while (run) {
      k = k + 1;

      printf("k = %d v1\n", k);

      if (isMobile) {
        cloud = Numeric::solve_RK4_movable_vortices(dt, grid, vortices, cloud);

        /* Decomposition de cloud qui est un std::vector<std::vector<double>> */

        dataVect.clear();
        for (std::size_t i = 0; i < vortices.numberOfVortices(); ++i) {
          dataVect.push_back(vortices.getCenter(i).x);
          dataVect.push_back(vortices.getCenter(i).y);
          dataVect.push_back(vortices.getIntensity(i));
        }
        for (std::size_t i = 0; i < cloud.size(); ++i) {
          dataVect.push_back(cloud[i].x);
          dataVect.push_back(cloud[i].y);
        }

        MPI_Send(&dataVect[0],
                 vortices.numberOfVortices() * 3 + cloud.size() * 2, MPI_DOUBLE,
                 0, 10, commGlob);

      } else {
        cloud = Numeric::solve_RK4_fixed_vortices(dt, grid, cloud);
      }
      printf("k = %d v2\n", k);
    }
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}