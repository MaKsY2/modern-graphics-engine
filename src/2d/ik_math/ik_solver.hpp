#ifndef IK_SOLVER
#define IK_SOLVER

#include <vector>
#include <SFML/Graphics.hpp>

class IKSolver
{
public:
    IKSolver();
    ~IKSolver();

    void fabrikSolve(std::vector<sf::Vector2f> &joints, const std::vector<float> &L, const sf::Vector2f &root, sf::Vector2f target, int maxIterations, float tol);
    void fabrikSolver3d(std::vector<sf::Vector3f> &joints, const std::vector<float> &L, const sf::Vector3f &root, sf::Vector3f target, int maxIterations, float tol);
};

#endif