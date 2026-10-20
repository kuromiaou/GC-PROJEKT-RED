#pragma once

// Near-plane triangle clipping (Sutherland-Hodgman) in CLIP space, before the
// perspective divide. A triangle straddling the near plane is recut into 1 or
// 2 triangles that are fully in front; every attribute is interpolated along
// the cut edges.
//
// Inside test (near plane is NDC z = -1):  z + w >= 0

struct ClipVertex
{
    float x, y, z, w;     // clip-space position (post-MVP, pre-divide)
    float u, v;           // texture coordinate
    float nx, ny, nz;     // world-space normal
};

namespace Clip
{
    inline ClipVertex Lerp(const ClipVertex& a, const ClipVertex& b, float t)
    {
        ClipVertex r;
        r.x  = a.x  + t * (b.x  - a.x);
        r.y  = a.y  + t * (b.y  - a.y);
        r.z  = a.z  + t * (b.z  - a.z);
        r.w  = a.w  + t * (b.w  - a.w);
        r.u  = a.u  + t * (b.u  - a.u);
        r.v  = a.v  + t * (b.v  - a.v);
        r.nx = a.nx + t * (b.nx - a.nx);
        r.ny = a.ny + t * (b.ny - a.ny);
        r.nz = a.nz + t * (b.nz - a.nz);
        return r;
    }

    /// Signed distance to the near plane; >= 0 means kept.
    inline float NearDist(const ClipVertex& v) { return v.z + v.w; }

    /// Writes 0, 3 or 6 vertices to out (0, 1 or 2 triangles); out holds >= 6.
    inline int ClipTriangleNear(const ClipVertex& v0, const ClipVertex& v1, const ClipVertex& v2,
                                ClipVertex* out)
    {
        const ClipVertex in[3] = { v0, v1, v2 };
        ClipVertex poly[4];
        int n = 0;

        for (int e = 0; e < 3; ++e)
        {
            const ClipVertex& a = in[e];
            const ClipVertex& b = in[(e + 1) % 3];
            const float da = NearDist(a);
            const float db = NearDist(b);
            const bool aIn = da >= 0.0f;
            const bool bIn = db >= 0.0f;

            if (aIn && bIn)
            {
                poly[n++] = b;
            }
            else if (aIn && !bIn)
            {
                poly[n++] = Lerp(a, b, da / (da - db));
            }
            else if (!aIn && bIn)
            {
                poly[n++] = Lerp(a, b, da / (da - db));
                poly[n++] = b;
            }
        }

        if (n == 3)
        {
            out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
            return 1;
        }
        if (n == 4)
        {
            out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
            out[3] = poly[0]; out[4] = poly[2]; out[5] = poly[3];
            return 2;
        }
        return 0;
    }
}
