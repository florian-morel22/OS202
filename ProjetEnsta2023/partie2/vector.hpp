#ifndef _GEOMETRY_VECTOR_HPP_
#define _GEOMETRY_VECTOR_HPP_
#include <string>
#include <cmath>
#include <stdexcept>
#include "point.hpp"

namespace Geometry
{
    /**
     * @brief Vector 2D
     * 
     * @tparam RealType The kind of real (float or double) 
     */
    template<typename RealType>
    struct Vector
    {
        RealType x,y;
        //@name Constructors and destructor
        //@{
        /**
         * @brief Default constructor
         * 
         */
        Vector() = default;
        /**
         * @brief Construct a vector with coordinates (t_x,t_y)
         * 
         * @param t_x Abcisse
         * @param t_y Ordinate
         */
        Vector( RealType t_x, RealType t_y )
            :   x(t_x), y(t_y)
        {}
        /**
         * @brief Construct a new Vector defined by two points
         * 
         * @param p1 The starting point
         * @param p2 The ending   point
         */
        Vector( Point<RealType> const& p1, Point<RealType> const& p2)
            :   x(p2.x-p1.x),
                y(p2.y-p1.y)
        {}
        /**
         * @brief Copy constructor
         * 
         */
        Vector( Vector const& ) = default;
        /**
         * @brief Move constructor
         * 
         */
        Vector( Vector     && ) = default;
        /**
         * @brief Destroy the Vector object
         * 
         */
        ~Vector()               = default;
        //@}
        //@name Operators
        //@{
        /**
         * @brief Default copy constructor
         * 
         * @return Vector& The current vector
         */
        Vector& operator = ( Vector const& ) = default;
        /**
         * @brief Default move construtor
         * 
         * @return Vector& The current vector
         */
        Vector& operator = ( Vector     && ) = default;
        /**
         * @brief Return the opposite of the current vector v : -v
         * 
         * @return Vector The opposite vector
         */
        Vector operator -() const 
        {
            return {-x,-y};
        }
        /**
         * @brief Return the scalar product of the current vector with u
         * 
         * @param u The second vector
         * @return double The scalar product
         */
        double operator | ( Vector const& u ) const
        {
            return x*u.x + y*u.y;
        }
        /**
         * @brief Addition of current vector with another vector
         * 
         * @param u  the second vector
         * @return Vector The result of the addition
         */
        Vector operator + ( Vector const& u )
        {
             return {x+u.x,y+u.y};
        }
        /**
         * @brief Return the difference between the current vector with a second vector
         * 
         * @param u The second vector
         * @return Vector The result of the operation
         */
        Vector operator - ( Vector const& u )
        {
            return {x-u.x,y-u.y};
        }
        /**
         * @brief Return a human readable string which represent the current vector
         *
         * Equivalent to the repr function in python
         * 
         * @return std::string 
         */
        explicit operator std::string() const
        {
            return std::string("(") + std::to_string(x) + "," + std::to_string(y) + ")";
        }
        //@}
        //@name Methods
        //@{
        /**
         * @brief Return the vector normalized (i.e ||u||=1)
         *
         * Throw an exception if the vector is null.
         * 
         * @return Vector The normalized vector
         */
        Vector normalize() const 
        {
            double nrm = normL2();
            if (nrm < 1.E-14)
                throw std::runtime_error("Normalize a null vector !");
            return {x/nrm,y/nrm};
        }
        /**
         * @brief Return the direct orthogonal vector relative to the current vector
         *
         * Used to compute the contribution velocity of each vortex
         * 
         * @return Vector The new vector
         */
        Vector computeDirectOrthogonalVector() const 
        {
            return {-y,x};
        }
        /**
         * @brief Return the square of euclidian norm of the vector
         * 
         * @return double $x^{2}+y^{2}$
         */
        double sqrNormL2() const 
        {
            return x*x+y*y;
        }
        /**
         * @brief Return the euclidian norm of the vector
         * 
         * @return double $\sqrt{x^{2}+y^{2}}$
         */
        double normL2() const 
        {
            return std::sqrt(sqrNormL2());
        }
        //@}
    };

    /**
     * @brief Compute the homothetie of a vector by a scalar 
     * 
     * @tparam RealType Kind of real
     * @param scalar    The scalar
     * @param u         The vector u
     * @return Vector<RealType> The result of the homothetie
     */
    template<typename RealType> Vector<RealType>
    operator * ( RealType scalar, Vector<RealType> const& u ) 
    {
        return { scalar*u.x, scalar*u.y};
    }

    /**
     * @brief Compute the translation of a point by a vector
     * 
     * @tparam RealType Kind of real
     * @param a_point   The point to translate
     * @param a_vector  The vector which define the translation
     * @return Point<RealType> The new point generated by the translation
     */
    template<typename RealType> Point<RealType>
    operator + ( Point<RealType> const& a_point, Vector<RealType> const& a_vector )
    {
        return {a_point.x+a_vector.x, a_point.y+a_vector.y};
    }
}

#endif