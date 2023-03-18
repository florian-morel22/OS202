#include "cartesian_grid_of_speed.hpp"
#include <cassert>
#include <iostream>

using namespace Numeric;

CartesianGridOfSpeed::CartesianGridOfSpeed(
    std::pair<std::size_t, std::size_t> t_dimensions,
    Geometry::Point<double> t_origin, double t_hStep)
    : m_width(t_dimensions.first), m_height(t_dimensions.second),
      m_left(t_origin.x), m_bottom(t_origin.y), m_step(t_hStep),
      m_velocityField(t_dimensions.first * t_dimensions.second) {
  assert(m_width > 0);
  assert(m_height > 0);
  assert(m_step > 0.);
}

void CartesianGridOfSpeed::updateVelocityField(
    Simulation::Vortices const &t_vortices) {
  using point = Simulation::Vortices::point;
  double halfStep = 0.5 * m_step;
  for (std::size_t iRow = 0; iRow < m_height; ++iRow) {
    double yP = m_bottom + iRow * m_step + halfStep;
    for (std::size_t jCol = 0; jCol < m_width; ++jCol) {
      std::size_t index = iRow * m_width + jCol;
      // Calcul de la coordonnée du centre de la cellule :
      point p{m_left + m_step * jCol + halfStep, yP};
      m_velocityField[index] = t_vortices.computeSpeed(p);
    }
  }
}

// A BESOIN DES CELLULES VOISINES DE P
auto CartesianGridOfSpeed::computeVelocityFor(point const &p) const -> vector {
  double halfStep = 0.5 * m_step;
  // Localise le point dans la grille cartésienne :
  std::int64_t iLoc = (p.x - m_left) / m_step;
  std::int64_t jLoc = (p.y - m_bottom) / m_step;
  point centerCell{getLeftBottomVertex().x + iLoc * m_step + halfStep,
                   getLeftBottomVertex().y + jLoc * m_step + halfStep};
  std::int64_t iRight = (iLoc + 1) % m_width; // Gestion du tore
  std::int64_t iLeft = (iLoc + m_width - 1) % m_width;

  std::int64_t jTop = (jLoc + 1) % m_height; // Gestion du tore
  std::int64_t jBot = (jLoc + m_height - 1) % m_height;

  // Interpolation quadratique :
  double invStep = 1. / m_step;
  double sqrStep = m_step * m_step;
  double invSqrStep = invStep * invStep;
  double invCubStep = invStep * invSqrStep;

  vector vcc = getVelocity(jLoc, iLoc);
  vector vcm = getVelocity(jLoc, iLeft);
  vector vcp = getVelocity(jLoc, iRight);
  vector vmc = getVelocity(jBot, iLoc);
  vector vpc = getVelocity(jTop, iLoc);
  // Polynôme :
  // v00 + v01.x + v10.y + v11.x.y + v02.x² + v20.y² + v12.y.x² + v21.y².x +
  // v22.x².y² = V Pour x=h, y=0 : v00 + v01.h + v02.h² = V0,+1 Pour x=-h,y=0 :
  // v00 - v01.h + v02.h² = V0,-1
  // => v01 = 1/(2h)(V0,+1 - V0,-1)
  // => v02 = (V0,+1 + V0,-1 - 2.v00)/(2.h²)
  vector v00 = vcc;
  vector v01 = (0.5 * invStep) * (vcp - vcm);
  vector v02 = (0.5 * invSqrStep) * (vcp + vcm - 2. * v00);
  // Même type de formule pour la direction Oy :
  vector v10 = (0.5 * invStep) * (vpc - vmc);
  vector v20 = (0.5 * invSqrStep) * (vpc + vmc - 2. * v00);
  // Pour x=h,y=h :
  //     v00 + v01.h + v10.h + v11.h² + v02.h² + v20.h² + v12.h^3 + v21.h^3 +
  //     v22.h^4 = V+1,+1
  // Pour x=-h,y=h :
  //     v00 - v01.h + v10.h - v11.h² + v02.h² + v20.h² + v12.h^3 - v21.h^3 +
  //     v22.h^4 = V-1,+1
  // Pour x=+h,y=-h :
  //     v00 + v01.h - v10.h - v11.h² + v02.h² + v20.h² - v12.h^3 + v21.h^3 +
  //     v22.h^4 = V+1,-1
  // Pour x=-h,y=-h :
  //     v00 - v01.h - v10.h + v11.h² + v02.h² + v20.h² - v12.h^3 - v21.h^3 +
  //     v22.h^4 = V-1,-1
  //
  // => 2v00 + 2v11.h² + 2v02.h² + 2v20.h² + 2v22.h^4 = V+1,+1 + V-1,-1
  // => 2v00 - 2v11.h² + 2v02.h² + 2v20.h² + 2v22.h^4 = V-1,+1 + V+1,-1
  // => 4v00 + 4v02.h² + 4v20.h² + 4v22.h^4 = V+1,+1 + V-1,-1 + V-1,+1 + V+1,-1
  // => v22 = (0.25/h^4)*(V+1,+1 + V-1,-1 + V-1,+1 + V+1,-1 - 4v00 - 4v02.h²
  // -4v20.h²)
  // => 4v11.h² = V+1,+1 + V-1,-1 - V-1,+1 + V+1,-1
  //
  // => 2v00  + 2v01.h + v02.h² + v20.h² + v21.h^3 + v22.h^4 = V+1,+1 + V+1,-1
  // =>

  // => 2v10.h + 2v11.h² + 2v12.h^3 = V+1,+1 - V+1,-1
  // => v12 = 0.5/h^3 * (V+1,+1 - V+1,-1 - 2v10.h -2v11.h²)
  //
  // => 2v01.h + 2v11.h^2 + 2v21.h^3 = V+1,+1 - V-1,+1
  // => v21 = 0.5/h^3 * ( V+1,+1 - V-1,+1 - 2v01.h - 2v11.h²)
  vector vmm = getVelocity(jBot, iLeft);
  vector vpm = getVelocity(jTop, iLeft);
  vector vmp = getVelocity(jBot, iRight);
  vector vpp = getVelocity(jTop, iRight);

  vector v11 = 0.25 * invSqrStep * (vmm + vpp - vmp - vpm);
  vector v22 = 0.25 * invSqrStep *
               (vmm + vmp + vpm + vpp - 4. * v00 - 4. * sqrStep * v02 -
                4. * sqrStep * v20);
  vector v12 =
      0.5 * invCubStep * (vpp - vpm - 2 * m_step * v10 - 2 * sqrStep * v11);
  vector v21 =
      0.5 * invCubStep * (vpp - vmp - 2 * m_step * v01 - 2 * sqrStep * v11);

  point locPoint{p.x - centerCell.x, p.y - centerCell.y};
  double xc = locPoint.x, yc = locPoint.y;
  double xc2 = xc * xc, yc2 = yc * yc;

  vector interpolatedVelocity{v00 + xc * v10 + yc * v01 + xc * yc * v11 +
                              xc2 * v20 + yc2 * v02 + xc2 * yc * v21 +
                              xc * yc2 * v12 + xc2 * yc2 * v22};
  return interpolatedVelocity;
}

auto CartesianGridOfSpeed::updatePosition(point const &p) const -> point {
  point newp(p);
  vector dimension(getLeftBottomVertex(), getRightTopVertex());
  if (newp.x < m_left)
    newp.x += dimension.x;
  if (newp.x > getRightTopVertex().x)
    newp.x -= dimension.x;
  if (newp.y < m_bottom)
    newp.y += dimension.y;
  if (newp.y > getRightTopVertex().y)
    newp.y -= dimension.y;
  return newp;
}
