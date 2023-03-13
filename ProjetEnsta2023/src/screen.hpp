#ifndef _GRAPHISM_SCREEN_HPP_
#define _GRAPHISM_SCREEN_HPP_
#include <utility>
#include <SFML/Window.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Graphics.hpp>

#include "cartesian_grid_of_speed.hpp"
#include "vortex.hpp"
#include "cloud_of_points.hpp"

namespace Graphisme
{
    class Screen
    {
    public:
        Screen( std::pair<std::size_t,std::size_t> const& t_geometry,
                std::pair<Geometry::Point<double>,Geometry::Point<double>> const& t_domain );

        bool isOpen() const 
        {
            return m_window.isOpen();
        }

        bool pollEvent( sf::Event& t_event )
        {
            return m_window.pollEvent(t_event);
        }

        void close()
        {
            m_window.close();
        }

        void displayVelocityField( Numeric::CartesianGridOfSpeed const& grid, Simulation::Vortices const& vortices );
        void displayParticles( Numeric::CartesianGridOfSpeed const& grid, Simulation::Vortices const& vortices, Geometry::CloudOfPoints const& points );

        void clear(sf::Color t_color)
        {
            m_window.clear(t_color);
        }

        void resize(sf::Event& event)
        {
            sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
            m_window.setView(sf::View(visibleArea));
        }

        std::pair<std::size_t,std::size_t> getGeometry() const 
        {
            return {m_window.getSize().x, m_window.getSize().y};
        }

        void drawText( std::string const& text, Geometry::Point<double> const& position);

        void display()
        {
            m_window.display();
        }
    private:
        sf::RenderWindow m_window;
        sf::Font   m_font  ;
        sf::View   m_velocityView, m_particlesView;
        sf::VertexArray m_grid; /// Grid display
        sf::VertexArray m_velocity; /// Velocity display
        sf::VertexArray m_particles; /// Particles display
    };
}

#endif