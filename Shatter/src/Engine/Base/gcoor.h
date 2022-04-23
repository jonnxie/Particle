//
// Created by maybe on 2021/7/9.
//

#ifndef SHATTER_ENGINE_GCOOR_H
#define SHATTER_ENGINE_GCOOR_H

#include <vulkan/vulkan.h>
#include <vector>
#include "Engine/Object/object.h"
class Line3d;

class GCoor : public Object {
public:
    static GCoor* createGCoor(const std::vector<glm::vec3>& _in);

public:
    GCoor(const std::vector<glm::vec3>& _in);
    void constructG() override;
    void constructD() override;
private:
    /*
     * index of line3d
     */
	std::vector<glm::vec3> lines;
};


#endif //SHATTER_ENGINE_GCOOR_H
