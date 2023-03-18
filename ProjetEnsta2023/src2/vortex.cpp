#include "vortex.hpp"
#include <iostream>

using namespace Simulation;

// On prend en compte 9 tourbillons pour prendre en compte la topologie du tore
// du domaine de cacul
auto Vortices::computeSpeed(point const &a_point) const -> vector {
  constexpr double thresholdDist = 1.E-5;
  vector speed{0., 0.};
  for (std::size_t iVortex = 0; iVortex < 3 * numberOfVortices();
       iVortex += 3) {

    point center{m_centers_and_intensities[iVortex + 0],
                 m_centers_and_intensities[iVortex + 1]};
    double intensity = m_centers_and_intensities[iVortex + 2];
    vector radius{center, a_point};
    double dist = radius.normL2();

    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    auto rightVortex = point{center.x + m_domainSize.x, center.y};
    radius = vector{rightVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    auto leftVortex = point{center.x - m_domainSize.x, center.y};
    radius = vector{leftVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    auto topVortex = point{center.x, center.y + m_domainSize.y};
    radius = vector{topVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    auto botVortex = point{center.x, center.y - m_domainSize.y};
    radius = vector{botVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    point topRightVortex{center.x + m_domainSize.x, center.y + m_domainSize.y};
    radius = vector{topRightVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    point topLeftVortex{center.x - m_domainSize.x, center.y + m_domainSize.y};
    radius = vector{topLeftVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    point botRightVortex{center.x + m_domainSize.x, center.y - m_domainSize.y};
    radius = vector{botRightVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }

    point botLeftVortex{center.x - m_domainSize.x, center.y - m_domainSize.y};
    radius = vector{botLeftVortex, a_point};
    dist = radius.normL2();
    if (dist > thresholdDist) {
      dist = std::max(dist, 1.);
      vector tangentVector = radius.computeDirectOrthogonalVector();
      vector speedContribution = (intensity / dist) * tangentVector.normalize();
      speed = speed + speedContribution;
    }
  }
  return speed;
}
