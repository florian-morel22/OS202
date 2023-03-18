#include <cmath>
#include "cloud_of_points.hpp"

Geometry::CloudOfPoints 
Geometry::generatePointsIn(std::size_t t_nbPoints, const Rectangle &t_area)
{
    std::size_t sqrtNbPoints = std::size_t(std::sqrt(t_nbPoints));
    std::size_t nbPointsY    = t_nbPoints/sqrtNbPoints;
    std::size_t nbPointsX    = sqrtNbPoints + (t_nbPoints%sqrtNbPoints > 0 ? 1 : 0);

    double dx = t_area.topRight.x - t_area.bottomLeft.x;
    double hx = dx/nbPointsX;

    double dy = t_area.topRight.y - t_area.bottomLeft.y;
    double hy = dy/nbPointsY;

    CloudOfPoints cloud{nbPointsX*nbPointsY};

    for ( std::size_t ix=0; ix<nbPointsX; ++ix)
    {
        for (std::size_t jy=0; jy<nbPointsY; ++jy)
        {
            cloud[ix + jy*nbPointsX] = Point<double>{t_area.bottomLeft.x+(ix+0.5)*hx, t_area.bottomLeft.y+(jy+0.5)*hy};
        }
    }
    return cloud;
}
