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
    Handle();
    virtual ~Handle();
    virtual void pushUI() = 0;
    virtual void loadFile(const std::string& _filename) = 0;
    virtual void destroy() const = 0;
public:
    [[nodiscard]] glm::vec3& getWorkCenter() const;
    [[nodiscard]] TargetPlane& getTargetPlane() const;
public:
    ClassProtectedElement(m_pipeline, std::string, Pipeline);
    ClassProtectedElement(m_sets, std::vector<std::string>, Sets);
    ClassProtectedElement(m_color, glm::vec3 , Color);
    ClassProtectedElement(m_localCoordiante, int, Coordinate);
    ClassElementInitial(m_textured, bool, Textured, false);
protected:
    std::string m_textureId{};
public:
    void setTextureId(const std::string &_in) {
        setTextured(true);
        m_textureId = _in;
    }
    std::string getTextureId() {
        return m_textureId;
    }
};


#endif //MAIN_HANDLE_H
