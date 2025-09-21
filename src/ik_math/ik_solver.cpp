#include "ik_solver.hpp"

double distance(const sf::Vector2<double> &v1, const sf::Vector2<double> &v2);
double norm(const sf::Vector2<double> &v);
sf::Vector2<double> normalize(const sf::Vector2<double> &v);

IKSolver::IKSolver()
{
}

IKSolver::~IKSolver()
{
}

void IKSolver::fabrikSolve(std::vector<sf::Vector2<double>> &joints, const std::vector<double> &L, const sf::Vector2<double> &root, sf::Vector2<double> target, int maxIterations, double tol)
{
    const int n = joints.size();
    if (n < 2)
        return;

    double totalLength = 0.0;
    for (auto l : L)
        totalLength += l;

    if (distance(root, target) > totalLength)
    {
        sf::Vector2<double> dir = normalize(target - root);
        joints[0] = root;
        for (int i = 1; i < n - 1; i++)
            joints[i + 1] = joints[i] + dir * L[i];
        return;
    }

    for (int it = 0; it < maxIterations; it++)
    {
        joints.back() = target;
        for (int i = n - 2; i >= 0; i--)
        {
            double r = distance(joints[i + 1], joints[i]);
            if (r < 1e-9f)
                continue;
            sf::Vector2<double> dir = (joints[i] - joints[i + 1]) / r;
            joints[i] = joints[i + 1] + dir * L[i];
        }

        joints.front() = root;
        for (int i = 0; i < n - 1; ++i)
        {
            double r = distance(joints[i + 1], joints[i]);
            if (r < 1e-9f)
                continue;
            sf::Vector2<double> dir = (joints[i + 1] - joints[i]) / r;
            joints[i + 1] = joints[i] + dir * L[i];
        }
        if (distance(joints.back(), target) <= tol)
            break;
    }
}

double norm(const sf::Vector2<double> &v)
{
    return std::sqrt(v.dot(v));
}

double distance(const sf::Vector2<double> &v1, const sf::Vector2<double> &v2)
{
    return norm(v1 - v2);
}

sf::Vector2<double> normalize(const sf::Vector2<double> &v)
{
    double n = norm(v);
    return (n > 1e-9f) ? v / n : sf::Vector2<double>(0, 0);
}