//
// Created by AnWell on 2022/2/10.
//

#ifndef PARTICLE_PARTICLE_H
#define PARTICLE_PARTICLE_H

#include <glm.hpp>
#include <vector>
#include "../Shatter_Base/points.h"
#include "../Shatter_Object/listener.h"


class DrawPoint : public Listener{
public:
    DrawPoint();
private:
    std::deque<std::unique_ptr<Points>> points;
};


#endif //PARTICLE_PARTICLE_H
