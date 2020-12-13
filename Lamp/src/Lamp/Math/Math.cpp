#include "lppch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Lamp::Math
{
bool DecomposeTransform(const glm::mat4& transform, glm::vec3& trans, glm::vec3& rot, glm::vec3& scale)
{
    using namespace glm;
    using T = float;

    mat4 localMatrix(transform);
    if (epsilonEqual(localMatrix[3][3], static_cast<float>(0), epsilon<T>()))
    {
        return false;
    }

    if (epsilonNotEqual(localMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(localMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(localMatrix[2][3], static_cast<T>(0), epsilon<T>()))
    {
        localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0);
        localMatrix[3][3] = static_cast<T>(1);
    }

    //Get the transform
    trans = vec3(localMatrix[3]);
    localMatrix[3] = vec4(0, 0, 0, localMatrix[3].w);

    vec3 row[3], Pdum3;

    //Scale and shear
    for (length_t i = 0; i < 3; ++i)
    {
        for (length_t j = 0; j < 3; ++j)
        {
            row[i][j] = localMatrix[i][j];
        }
    }

    scale.x = length(row[0]);
    row[0] = detail::scale(row[0], static_cast<T>(1));
    scale.y = length(row[1]);
    row[1] = detail::scale(row[1], static_cast<T>(1));
    scale.z = length(row[2]);
    row[2] = detail::scale(row[2], static_cast<T>(1));

    rot.y = asin(-row[0][2]);
    if (cos(rot.y) != 0)
    {
        rot.x = atan2(row[1][2], row[2][2]);
        rot.z = atan2(row[0][1], row[0][0]);
    }
    else
    {
        rot.x = atan2(-row[2][0], row[1][1]);
        rot.z = 0;
    }

    return true;
}
bool SolveQuadric(const float& a, const float& b, const float& c, float& x0, float& x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0)
    {
        return false;
    }
    else if (discr == 0)
    {
        x0 = x1 = -0.5f * b / a;
    }
    else
    {
        float q = (b > 0) ?
                  -0.5f * (b + sqrt(discr)) :
                  -0.5f * (b - sqrt(discr));

        x0 = q / a;
        x1 = c / q;
    }

    return true;
}
float Max(const glm::vec3& v)
{
    float max = -FLT_MAX;

    if (v.x > max)
    {
        max = v.x;
    }

    if (v.y > max)
    {
        max = v.y;
    }

    if (v.z > max)
    {
        max = v.z;
    }

    return max;
}
}