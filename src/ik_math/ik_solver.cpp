#include "ik_solver.hpp"

float distance(const sf::Vector2<float> &v1, const sf::Vector2<float> &v2);
float norm(const sf::Vector2<float> &v);
sf::Vector2<float> normalize(const sf::Vector2<float> &v);

IKSolver::IKSolver()
{
}

IKSolver::~IKSolver()
{
}

void IKSolver::fabrikSolve(std::vector<sf::Vector2f> &joints, const std::vector<float> &L, const sf::Vector2f &root, sf::Vector2f target, int maxIterations, float tol)
{
    const int n = joints.size();
    if (n < 2)
        return;

    float totalLength = 0.0;
    for (auto l : L)
        totalLength += l;

    if (distance(root, target) > totalLength)
    {
        sf::Vector2<float> dir = normalize(target - root);
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
            float r = distance(joints[i + 1], joints[i]);
            if (r < 1e-9f)
                continue;
            sf::Vector2<float> dir = (joints[i] - joints[i + 1]) / r;
            joints[i] = joints[i + 1] + dir * L[i];
        }

        joints.front() = root;
        for (int i = 0; i < n - 1; ++i)
        {
            float r = distance(joints[i + 1], joints[i]);
            if (r < 1e-9f)
                continue;
            sf::Vector2<float> dir = (joints[i + 1] - joints[i]) / r;
            joints[i + 1] = joints[i] + dir * L[i];
        }
        if (distance(joints.back(), target) <= tol)
            break;
    }
}

float norm(const sf::Vector2<float> &v)
{
    return std::sqrt(v.dot(v));
}

float distance(const sf::Vector2<float> &v1, const sf::Vector2<float> &v2)
{
    return norm(v1 - v2);
}

sf::Vector2<float> normalize(const sf::Vector2<float> &v)
{
    float n = norm(v);
    return (n > 1e-9f) ? v / n : sf::Vector2<float>(0, 0);
}