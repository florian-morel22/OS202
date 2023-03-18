#ifndef _POINT_HPP_
#define _POINT_HPP_
#include <cmath>
#include <cstddef>
#include <string>

namespace Geometry {
/**
 * @brief A simple 2D point on the plane
 *
 * @tparam RealType The kind of real representing the coordinates of the point
 */
template <typename RealType> struct Point {
  RealType x, y;

  //@name Constructors and destructor
  //@{
  /**
   * @brief Default Constructor
   *
   */
  Point() = default;
  /**
   * @brief Construct a point at coordinate (t_x,t_y)
   *
   * @param t_x The abcisse
   * @param t_y The ordinate
   */
  Point(RealType t_x, RealType t_y) : x(t_x), y(t_y) {}
  /**
   * @brief Copy constructor
   *
   */
  Point(Point const &) = default;
  /**
   * @brief Movable constructor
   *
   */
  Point(Point &&) = default;
  /**
   * @brief Destroy the Point object
   *
   */
  ~Point() = default;
  //@}

  //@name Operators
  //@{
  /**
   * @brief Copy constructor
   *
   * @return Point& The point to copy
   */
  Point &operator=(Point const &) = default;
  /**
   * @brief Movable constructor
   *
   * @return Point& The move constructor
   */
  Point &operator=(Point &&) = default;
  /**
   * @brief Convert the point in human readable string (for output on terminal
   * or file)
   *
   * This convertion operator is equivalent at the repr function in python
   * language.
   *
   * @return std::string
   */

  // Point &operator[](std::size_t t_index) { return t_index == 0 ? x : y; }

  explicit operator std::string() const {
    return std::string("{") + std::to_string(x) + "," + std::to_string(y) + "}";
  }
  //@}

  //@name Methods
  //@{
  /**
   * @brief Compute the cartesian distance between the current point and a point
   * p
   *
   * @param p The other point
   * @return double The cartesian distance between current point and p
   */
  double computeDistance(Point const &p) const {
    double dx = x - p.x;
    double dy = y - p.y;
    return std::sqrt(dx * dx + dy * dy);
  }
  //@}
};
} // namespace Geometry

#endif