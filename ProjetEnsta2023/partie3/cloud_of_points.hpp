#ifndef _GEOMETRY_CLOUD_OF_POINTS_HPP_
#define _GEOMETRY_CLOUD_OF_POINTS_HPP_
#include "point.hpp"
#include "rectangle.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

namespace Geometry {
/**
 * @brief A set of points in the plane $\mathbb{R}^{2}$
 *
 */
class CloudOfPoints {
public:
  using point = Point<double>;
  using container = std::vector<point>;
  using iterator = container::iterator;
  using const_iterator = container::const_iterator;

  //@name Constructors and destructor
  //@{

  CloudOfPoints() = default;
  CloudOfPoints(std::size_t nbPoints) : m_setOfPoints(nbPoints) {}
  CloudOfPoints(std::vector<double> const &m_coordinates);
  CloudOfPoints(CloudOfPoints const &) = default;
  CloudOfPoints(CloudOfPoints &&) = default;
  ~CloudOfPoints() = default;
  //@}
  point const &operator[](std::size_t t_index) const {
    assert(t_index < m_setOfPoints.size());
    return m_setOfPoints[t_index];
  }

  point &operator[](std::size_t t_index) {
    assert(t_index < m_setOfPoints.size());
    return m_setOfPoints[t_index];
  }

  iterator begin() { return m_setOfPoints.begin(); }
  const_iterator begin() const { return m_setOfPoints.cbegin(); }
  const_iterator cbegin() const { return m_setOfPoints.cbegin(); }

  iterator end() { return m_setOfPoints.end(); }
  const_iterator end() const { return m_setOfPoints.cend(); }
  const_iterator cend() const { return m_setOfPoints.cend(); }

  std::size_t numberOfPoints() const { return m_setOfPoints.size(); }

  double const *data() const { return (double *)m_setOfPoints.data(); }

  double *data() { return (double *)m_setOfPoints.data(); }

  void removeAPoint(std::size_t t_index) {
    assert(t_index < numberOfPoints());
    std::swap(m_setOfPoints[t_index], m_setOfPoints.back());
    m_setOfPoints.pop_back();
  }

  void addAPoint(point const &a_point) { m_setOfPoints.push_back(a_point); }

  CloudOfPoints &operator=(CloudOfPoints const &) = default;
  CloudOfPoints &operator=(CloudOfPoints &&) = default;

  size_t size() const { return m_setOfPoints.size(); }

private:
  container m_setOfPoints;
};

CloudOfPoints generatePointsIn(std::size_t t_nbPoints, Rectangle const &t_area);
} // namespace Geometry

#endif
