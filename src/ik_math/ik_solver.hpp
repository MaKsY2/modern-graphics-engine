#ifndef IK_SOLVER
#define IK_SOLVER

#include <vector>
#include <SFML/Graphics.hpp>

class IKSolver
{
public:
    IKSolver();
    ~IKSolver();

    void fabrikSolve(std::vector<sf::Vector2<double>> &joints, const std::vector<double> &L, const sf::Vector2<double> &root, sf::Vector2<double> target, int maxIterations = 32, double tol = 1e-3f);
};

#endif