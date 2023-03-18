#include "screen.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <iostream>
#include <locale>
#include <cassert>


Graphisme::Screen::Screen( std::pair<std::size_t,std::size_t> const& t_geometry,
                           std::pair<Geometry::Point<double>,Geometry::Point<double>> const& t_domain )
    :   m_window(sf::VideoMode(t_geometry.first, t_geometry.second), "Vortex simulation")
{
    if (!m_font.loadFromFile("data/Aller_Bd.ttf"))
    {
        throw std::ios_base::failure("Failed to load font file Aller_Bd.ttf");
    }
    auto screenSize = m_window.getSize();
    m_velocityView.setCenter(0.25*screenSize.x,0.5*(screenSize.y-128));
    m_velocityView.setSize  (0.5*screenSize.x, screenSize.y-128);
    m_velocityView.setViewport(sf::FloatRect(0.01f,0.05f,0.48f,0.8f));

    m_particlesView.setCenter(0.25*screenSize.x,0.5*(screenSize.y-128));
    m_particlesView.setSize  (0.5*screenSize.x, screenSize.y-128);
    m_particlesView.setViewport(sf::FloatRect(0.51f,0.05f,0.48f,0.8f));
}

void 
Graphisme::Screen::displayVelocityField( Numeric::CartesianGridOfSpeed const& grid, Simulation::Vortices const& vortices )
{
    using vector=Numeric::CartesianGridOfSpeed::vector;
    m_window.setView(m_velocityView);
    // Affichage moité gauche de l'écran : 
    auto screenSize = m_velocityView.getSize();
    std::size_t width = screenSize.x-1;
    std::size_t height= screenSize.y-1;

    vector domainDimension{grid.getLeftBottomVertex(),grid.getRightTopVertex()};

    auto nbCells = grid.cellGeometry();
    double scalex = width/domainDimension.x;
    double scaley = height/domainDimension.y;
    double hx     = grid.getStep()*scalex;
    double hy     = grid.getStep()*scaley;

    if (m_grid.getVertexCount() == 0)
    {
        m_grid= sf::VertexArray(sf::Lines, 2*(nbCells.first+nbCells.second+2));
        std::size_t indDrawVert = 0;
        for (std::size_t ix=0; ix<=nbCells.first; ++ix)
        {
            m_grid[indDrawVert] = sf::Vertex(sf::Vector2f(ix*hx, 0));
            m_grid[indDrawVert].color = sf::Color(64,0,0);
            ++indDrawVert;
            m_grid[indDrawVert] = sf::Vertex(sf::Vector2f(ix*hx, height));
            m_grid[indDrawVert].color = sf::Color(64,0,0);
            ++indDrawVert;
        }
        for (std::size_t jy=0; jy<=nbCells.second; ++jy)
        {
            m_grid[indDrawVert] = sf::Vertex(sf::Vector2f(0,jy*hy));
            m_grid[indDrawVert].color = sf::Color(64,0,0);
            ++indDrawVert;
            m_grid[indDrawVert] = sf::Vertex(sf::Vector2f(width, jy*hy));
            m_grid[indDrawVert].color = sf::Color(64,0,0);
            ++indDrawVert;
        } 
    }

    if (m_velocity.getVertexCount() == 0)
    {
        m_velocity = sf::VertexArray(sf::Lines, 2*nbCells.first*nbCells.second);
    }

    // Affichage des vecteurs :
    for (std::size_t jy=0; jy<nbCells.second; ++jy)
    {
        for (std::size_t ix=0; ix<nbCells.first; ++ix)
        {
            std::size_t indDrawVert = 2*(jy*nbCells.first + ix);
            double xdeb = (ix+0.5)*hx;
            double ydeb = (jy+0.5)*hy;
            double xfin = xdeb + scalex*grid.getVelocity(jy, ix).x;
            double yfin = ydeb + scaley*grid.getVelocity(jy, ix).y;
            m_velocity[indDrawVert] = sf::Vertex(sf::Vector2f(xdeb,ydeb));
            m_velocity[indDrawVert].color = sf::Color(0,0,127);
            ++indDrawVert;
            m_velocity[indDrawVert] = sf::Vertex(sf::Vector2f(xfin,yfin));
            m_velocity[indDrawVert].color = sf::Color(255,255,255);
            ++indDrawVert;
        }
    }
    m_window.draw(m_grid);
    m_window.draw(m_velocity);
    // Affichage des vortices :
    for (std::size_t iVort=0; iVort<vortices.numberOfVortices(); ++iVort)
    {
        auto c = vortices.getCenter(iVort);
        sf::CircleShape shape{5};
        shape.setPosition(scalex*(c.x-grid.getLeftBottomVertex().x)-5,scaley*(c.y-grid.getLeftBottomVertex().y)-5);
        shape.setFillColor(sf::Color::Red);
        m_window.draw(shape);
    }
    m_window.setView(m_window.getDefaultView());
}
//-----------------------------------------------------------------------------------------------------------
void 
Graphisme::Screen::displayParticles( Numeric::CartesianGridOfSpeed const& grid, Simulation::Vortices const& vortices, Geometry::CloudOfPoints const& points )
{
    using vector=Numeric::CartesianGridOfSpeed::vector;
    m_window.setView(m_particlesView);
    // Affichage moité gauche de l'écran : 
    auto screenSize = m_particlesView.getSize();
    std::size_t width = screenSize.x-1;
    std::size_t height= screenSize.y-1;

    vector domainDimension{grid.getLeftBottomVertex(),grid.getRightTopVertex()};

    double scalex = width/domainDimension.x;
    double scaley = height/domainDimension.y;


    if (m_particles.getVertexCount()==0)
    {
        m_particles = sf::VertexArray(sf::Points, points.numberOfPoints());
    }
    assert(m_particles.getVertexCount() == points.numberOfPoints());
    // Affichage des particules en 3/4 transparents :
    
    for (std::size_t iPt=0; iPt<points.numberOfPoints(); ++iPt)
    {
        m_particles[iPt].position = sf::Vector2f{
            float(scalex*(points[iPt].x-grid.getLeftBottomVertex().x)),
            float(scaley*(points[iPt].y-grid.getLeftBottomVertex().y))};
        m_particles[iPt].color = sf::Color(255,255,255,128);
    }
    m_window.draw(m_particles);
    // Affichage des vortices :
    for (std::size_t iVort=0; iVort<vortices.numberOfVortices(); ++iVort)
    {
        auto c = vortices.getCenter(iVort);
        sf::CircleShape shape{5};
        shape.setPosition(scalex*(c.x-grid.getLeftBottomVertex().x)-5,scaley*(c.y-grid.getLeftBottomVertex().y)-5);
        shape.setFillColor(sf::Color::Red);
        m_window.draw(shape);
    }
    m_window.setView(m_window.getDefaultView());
}
//
void 
Graphisme::Screen::drawText( std::string const& t_string, Geometry::Point<double> const& position)
{
    static sf::Text text;
    text.setFont(m_font);
    text.setString(t_string);
    text.setCharacterSize(12);
    text.setFillColor(sf::Color::White);
    text.setPosition(position.x,position.y);
    m_window.draw(text);
}
