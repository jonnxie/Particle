//
// Created by jonnxie on 2022/4/22.
//

#ifndef MAIN_CAPTUREOBJECT_H
#define MAIN_CAPTUREOBJECT_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_item.h"

class Object;

class AABB;

class AABBLine;

class CaptureObject {
public:
    static std::shared_ptr<CaptureObject> mallocCapture(Object* _parent,
                                                        const glm::vec3& _min,
                                                        const glm::vec3& _max,
                                                        int _drawId,
                                                        int _modelId,
                                                        const std::string& _name = "default");


    CaptureObject(Object* _parent, int _boxId, int _drawId, int _modelId, const std::string& _name);

    CaptureObject(Object* _parent, uint32_t _captureId, int _boxId);
    ~CaptureObject();
    DefineUnCopy(CaptureObject);
public:
    ClassElementInitial(color, glm::vec3, Color, CYAN_COLOR);
    ClassPointerElement(captureId, uint32_t, CaptureId);
    ClassPointerElement(boxId, int, BoxId);
    ClassPointerElement(parent, Object* , Parent);
    void drawBox();
    void hide();
private:
    std::unique_ptr<AABBLine>   line{nullptr};
    std::string                 bufferId;
};


#endif //MAIN_CAPTUREOBJECT_H
