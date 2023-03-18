////////////////////////////////////
// parallélisation du calcul du déplacement des points
// Fonctionne pour isMobile = 0 et isMobile = 1
////////////////////////////////////

#include "cartesian_grid_of_speed.hpp"
#include "cloud_of_points.hpp"
#include "point.hpp"
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
  int nbp, rank, flag;
  MPI_Init(&nargs, &argv);
  printf("nargs = %d\n", nargs);
  printf("argv[4] = %s\n", *argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &commGlob);
  MPI_Comm_size(commGlob, &nbp);
  MPI_Comm_rank(commGlob, &rank);
  MPI_Status status;

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
  bool advance = false;
  bool run_calcul = true;
  double dt = 0.1;

  bool receive_data = false;

  int sizeBuffers = cloud.numberOfPoints() / (nbp - 1);
  int sizeLastBuffers = cloud.numberOfPoints() - (nbp - 2) * sizeBuffers;
  std::vector<double> partialDataVect;
  partialDataVect.resize(sizeBuffers * 2);

  std::vector<double> gridVect;
  gridVect.resize(grid.size() * 2);

  MPI_Request req;

  if (rank == 0) {
    Graphisme::Screen myScreen(
        {resx, resy}, {grid.getLeftBottomVertex(), grid.getRightTopVertex()});

    while (myScreen.isOpen()) {
      auto start = std::chrono::system_clock::now();
      advance = false;
      // on inspecte tous les évènements de la fenêtre qui ont été émis depuis
      // la précédente itération

      /* EVENT */
      sf::Event event;
      while (myScreen.pollEvent(event)) {
        // évènement "fermeture demandée"+ : on ferme la fenêtre
        if (event.type == sf::Event::Closed) {
          myScreen.close();
          run_calcul = false;
          animate = false;
          advance = false;
          for (int iProc = 1; iProc < nbp; ++iProc) {
            MPI_Send(&run_calcul, 1, MPI_LOGICAL, iProc, 20, commGlob);
          }
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
          for (int iProc = 1; iProc < nbp; ++iProc) {
            MPI_Send(&dt, 1, MPI_DOUBLE, iProc, 22, commGlob);
          }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
          dt /= 2;
          for (int iProc = 1; iProc < nbp; ++iProc) {
            MPI_Send(&dt, 1, MPI_DOUBLE, iProc, 22, commGlob);
          }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
          advance = true;
          MPI_Send(&advance, 1, MPI_LOGICAL, 1, 23, commGlob);
        }
      }

      /* CALCUL */

      receive_data = animate | advance;

      if (receive_data) {

        gridVect.clear();
        for (std::size_t i = 0; i < grid.size(); ++i) {
          gridVect.push_back(grid[i].x);
          gridVect.push_back(grid[i].y);
        }

        // On demande aux processus de calcul d'envoyer leurs données
        for (int i = 1; i < nbp; ++i) {
          MPI_Isend(&gridVect[0], grid.size() * 2, MPI_DOUBLE, i, 11, commGlob,
                    &req);
        }
        MPI_Wait(&req, MPI_STATUS_IGNORE);

        partialDataVect.clear();
        partialDataVect.resize(sizeBuffers * 2);
        for (int iProc = 1; iProc < nbp - 1; ++iProc) {

          MPI_Recv(&partialDataVect[0], sizeBuffers * 2, MPI_DOUBLE, iProc, 10,
                   commGlob, MPI_STATUS_IGNORE);

          for (int iPoint = 0; iPoint < sizeBuffers; ++iPoint) {
            int p = (iProc - 1) * sizeBuffers + iPoint;

            cloud[p].x = partialDataVect[2 * iPoint];
            cloud[p].y = partialDataVect[2 * iPoint + 1];
          }
        }

        partialDataVect.resize(sizeLastBuffers * 2);
        MPI_Recv(&partialDataVect[0], sizeLastBuffers * 2, MPI_DOUBLE, nbp - 1,
                 10, commGlob, MPI_STATUS_IGNORE);

        for (int iPoint = 0; iPoint < sizeLastBuffers; ++iPoint) {
          int p = (nbp - 2) * sizeBuffers + iPoint;

          cloud[p].x = partialDataVect[2 * iPoint];
          cloud[p].y = partialDataVect[2 * iPoint + 1];
        }

        Numeric::Calcul_Vortexs_VelocityField(dt, grid, vortices);
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

  else if (rank == nbp - 1) {

    partialDataVect.resize(sizeLastBuffers * 2);

    Geometry::CloudOfPoints partialCloud(sizeLastBuffers);
    for (int iPoint = 0; iPoint < sizeLastBuffers; ++iPoint) {
      partialCloud[iPoint] = cloud[sizeBuffers * (rank - 1) + iPoint];
    }

    while (run_calcul) {

      MPI_Iprobe(0, 20, commGlob, &flag, &status);
      if (flag) {
        MPI_Recv(&run_calcul, 1, MPI_LOGICAL, 0, 20, commGlob, &status);
        break;
      }

      MPI_Iprobe(0, 22, commGlob, &flag, &status);
      if (flag) {
        MPI_Recv(&dt, 1, MPI_DOUBLE, 0, 22, commGlob, &status);
      }

      MPI_Iprobe(0, 11, commGlob, &flag, &status);
      if (flag) {
        MPI_Recv(&gridVect[0], grid.size() * 2, MPI_DOUBLE, 0, 11, commGlob,
                 &status);

        for (std::size_t i = 0; i < grid.size(); ++i) {
          grid[i].x = gridVect[2 * i];
          grid[i].y = gridVect[2 * i + 1];
        }

        partialCloud =
            Numeric::solve_RK4_fixed_vortices(dt, grid, partialCloud);
        partialDataVect.clear();
        for (std::size_t i = 0; i < partialCloud.numberOfPoints(); ++i) {
          partialDataVect.push_back(partialCloud[i].x);
          partialDataVect.push_back(partialCloud[i].y);
        }

        MPI_Send(&partialDataVect[0], sizeLastBuffers * 2, MPI_DOUBLE, 0, 10,
                 commGlob);
      }
    }
  } else {

    Geometry::CloudOfPoints partialCloud(sizeBuffers);
    for (int iPoint = 0; iPoint < sizeBuffers; ++iPoint) {
      partialCloud[iPoint] = cloud[sizeBuffers * (rank - 1) + iPoint];
    }

    while (run_calcul) {

      MPI_Iprobe(0, 20, commGlob, &flag, &status);
      if (flag) {
        MPI_Recv(&run_calcul, 1, MPI_LOGICAL, 0, 20, commGlob, &status);
        break;
      }

      MPI_Iprobe(0, 22, commGlob, &flag, &status);
      if (flag) {
        MPI_Recv(&dt, 1, MPI_DOUBLE, 0, 22, commGlob, &status);
      }

      MPI_Iprobe(0, 11, commGlob, &flag, &status);
      if (flag) {
        MPI_Recv(&gridVect[0], grid.size() * 2, MPI_DOUBLE, 0, 11, commGlob,
                 &status);

        for (std::size_t i = 0; i < grid.size(); ++i) {
          grid[i].x = gridVect[2 * i];
          grid[i].y = gridVect[2 * i + 1];
        }

        partialCloud =
            Numeric::solve_RK4_fixed_vortices(dt, grid, partialCloud);
        partialDataVect.clear();
        for (std::size_t i = 0; i < partialCloud.numberOfPoints(); ++i) {
          partialDataVect.push_back(partialCloud[i].x);
          partialDataVect.push_back(partialCloud[i].y);
        }

        MPI_Send(&partialDataVect[0], sizeBuffers * 2, MPI_DOUBLE, 0, 10,
                 commGlob);
      }
    }
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}