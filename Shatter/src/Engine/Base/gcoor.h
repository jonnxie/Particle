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
    static GCoor* createGCoor(const std::vector<int>& _in);
    static GCoor* createGCoor(int _x,int _y,int _z);

public:
    GCoor(int _x,int _y,int _z):m_x_axis(_x),m_y_axis(_y),m_z_axis(_z){};
    explicit GCoor(const std::vector<int>& _in);
    void constructG() override;
    void constructD() override;
private:
    /*
     * index of line3d
     */
    int m_x_axis;
    int m_y_axis;
    int m_z_axis;
};


#endif //SHATTER_ENGINE_GCOOR_H
