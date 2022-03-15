//
// Created by jonnxie on 2022/3/15.
//

#ifndef MAIN_COMPONENT_H
#define MAIN_COMPONENT_H

#include <glm.hpp>

class Component {

};

struct Transform{
    glm::mat4 transition;
    glm::mat4 rotate;
    glm::mat4 scale;
};


#endif //MAIN_COMPONENT_H
