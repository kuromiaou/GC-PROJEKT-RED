#include "Maths.h"
#include "Mesh.h"

struct TriangleJob {
    Point p0, p1, p2;
    float z0, z1, z2;
    float iw0, iw1, iw2;
    UV uv0, uv1, uv2;
    Maths::Vec3f n0, n1, n2;
    Texture* texture;
    ///DirectionalLight light; to finish
    Maths::Vec3f halfVector;
};

std::array<std::binary_semaphore, 4> m_workerStart{
       std::binary_semaphore{0}, std::binary_semaphore{0},
       std::binary_semaphore{0}, std::binary_semaphore{0}
};
std::counting_semaphore<4> m_endSemaphore{ 0 };

std::vector<TriangleJob> m_jobs; //triangle temp