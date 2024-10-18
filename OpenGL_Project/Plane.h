#ifndef PLANE_H
#define PLANE_H

#include "ModelLoader.h"

class Plane : public ModelLoader
{
public:
    Plane();
    ~Plane() = default;
    void loadPlane();
};

#endif
