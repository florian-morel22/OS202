#ifndef _GEOMETRY_RECTANGLE_HPP_
#define _GEOMETRY_RECTANGLE_HPP_
#include "point.hpp"

namespace Geometry
{
    struct Rectangle 
    {
        Point<double> bottomLeft, topRight; 
        Rectangle( Point<double> const& t_botleft, Point<double> const& t_topRight )
            :   bottomLeft{t_botleft},
                topRight{t_topRight}
        {}
        Rectangle( Rectangle const& ) = default;
        Rectangle( Rectangle     && ) = default;
        ~Rectangle() = default;

        Rectangle& operator = ( Rectangle const& ) = default;
        Rectangle& operator = ( Rectangle     && ) = default;
    };
}

#endif