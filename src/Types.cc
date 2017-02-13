/*
    Types.cc
*/

#include "Types.h"

BasicVertex::BasicVertex(   float px, float py, float pz,
                            float ux, float uy)
{
    Position = glm::vec3(px, py, pz);
    Uv = glm::vec2(ux, uy);
}

void Frustrum::SetCamProjection(float angle, float aspect, float near, float far)
{
    mAspect = aspect;
    mNear = near;
    mFar = far;

    // compute width and height of the near and far plane sections
    mTang = tan(angle);
    mSphereFactorY = 1.0f / cos(angle);

    // compute half of the the horizontal field of view and sphereFactorX
    float anglex = atan(mTang*mAspect);
    mSphereFactorX = 1.0f / cos(anglex);
}

void Frustrum::SetCamVectors(glm::vec3 pos, glm::vec3 look, glm::vec3 up)
{
    mCamPosition = pos;
    mCamLook = glm::normalize(look - pos); 
    mCamRight = glm::normalize(glm::cross(mCamLook, up));
    mCamUp = glm::normalize(glm::cross(mCamRight, mCamLook));
}

FrustrumResult Frustrum::SphereInFrustrum(BoundingSphere & bs)
{
    float d;
    float az, ax, ay;
    FrustrumResult result = kInside;

    glm::vec3 v = bs.Position - mCamPosition;
   
    az = v.x * -mCamLook.x + v.y * -mCamLook.y + v.z * -mCamLook.z;
    if (az > mFar + bs.Radius || az < mNear - bs.Radius)
        return kOutside;

    if (az > mFar - bs.Radius || az < mNear + bs.Radius)
        result = kIntersect;

    ay = v.x * mCamUp.x + v.y * mCamUp.y + v.z * mCamUp.z;
    d = mSphereFactorY * bs.Radius;
    az *= mTang;
    if (ay > az + d || ay < -az - d)
        return kOutside;

    if (ay > az - d || ay < -az + d)
        result = kIntersect;

    ax = v.x * mCamLook.x + v.y * mCamLook.y + v.z * mCamLook.z;
    az *= mAspect;
    d = mSphereFactorX * bs.Radius;
    if (ax > az + d || ax < -az - d)
        return kOutside;

    if (ax > az - d || ax < -az + d)
        result = kIntersect;

    return result;
}

FrustrumResult Frustrum::PointInFrustrum(glm::vec3 p)
{
    float pcz, pcx, pcy, aux;

    // compute vector from camera position to p
    glm::vec3 v = p - mCamPosition;

    // compute and test the Z coordinate
    pcz = v.x * -mCamLook.x + v.y * -mCamLook.y + v.z * -mCamLook.z;
    if (pcz > mFar || pcz < mNear)
        return kOutside;

    // compute and test the Y coordinate
    pcy = v.x * mCamUp.x + v.y * mCamUp.y + v.z * mCamUp.z;
    aux = pcz * mTang;
    if (pcy > aux || pcy < -aux)
        return kOutside;;

    // compute and test the X coordinate
    pcx = v.x * mCamRight.x + v.y * mCamRight.y + v.z * mCamRight.z;
    aux = aux * mAspect;
    if (pcx > aux || pcx < -aux)
        return kOutside;

    return kInside;
}

void LoadMeshFromFile(const char * file, MeshBasicVertexData& md)
{
}

void GenerateSphere(float radius, int div, MeshBasicVertexData & md)
{
    float const R = 1.0f / (float)(div - 1);
    float const S = 1.0f / (float)(div - 1);
    float M_PI = 3.1415f;
    float M_PI_2 = M_PI * 0.5f;

    int r, s;
    float ux, uy;
    float px, py, pz;

    // Vertex data
    for (r = 0; r < div; r++)
    {
        for (s = 0; s < div; s++)
        {
            float const y = sin(-M_PI_2 + M_PI * r * R);
            float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
            float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);
            // Texcoord
            ux = s*S;
            uy = r*R;
            // Vertex
            px = x * radius;
            py = y * radius;
            pz = z * radius;
            // Normal
            //*n++ = x;
            //*n++ = y;
            //*n++ = z;
            md.vertex.push_back(BasicVertex(px, py, pz, ux, uy));
        }
    } 

    // Index
    for (r = 0; r < div; r++)
    {
        for (s = 0; s < div; s++)
        {
            md.ele.push_back(r * div + s);
            md.ele.push_back(r * div + (s + 1));
            md.ele.push_back((r + 1) * div + (s + 1));
            md.ele.push_back((r + 1) * div + s);
        }
    }
}

