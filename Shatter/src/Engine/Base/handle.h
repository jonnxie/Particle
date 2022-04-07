//
// Created by jonnxie on 2022/4/7.
//

#ifndef MAIN_HANDLE_H
#define MAIN_HANDLE_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_item.h"

class Handle {
public:
    Handle() = default;
    virtual ~Handle();
    virtual void pushUI();
    virtual void loadFile(const std::string& _filename);
    virtual void destroy() const;
public:
    [[nodiscard]] glm::vec3& getWorkCenter() const;
    [[nodiscard]] TargetPlane& getTargetPlane() const;
public:
    ClassElementInitial(m_pipeline, std::string, Pipeline, "Polyline");
    ClassElementInitial(m_sets, std::vector<std::string>, Sets, "Camera");
    ClassElement(m_color, glm::vec3 , Color);
    ClassElement(m_localCoordiante, int, Coordinate);
private:

};


#endif //MAIN_HANDLE_H
