//
// Created by jonnxie on 2022/5/18.
//

#include "operator.h"
#include <utility>
#include "Engine/Object/object.h"
#include AppCatalog
#include SceneCatalog

void pushObject2MainScene(int _id, SP(Object) _obj) {
    SingleAPP.mainScene->pushObject(_id, std::move(_obj));
}

void pushDraw2MainScene(int _id) {
    SingleAPP.mainScene->defaultPush(_id);
}

void pushTrans2MainScene(int _id) {
    SingleAPP.mainScene->transparencyPush(_id);

}

void pushNorm2MainScene(int _id) {
    SingleAPP.mainScene->normalPush(_id);

}

void pushCapture2MainScene(int _boxId, int _captureId) {
    SingleAPP.mainScene->addAABB(_captureId, _boxId);
}

void releaseObject2MainScene(int _id, std::shared_ptr<Object> _obj) {
    SingleAPP.mainScene->releaseObject(_id);
}

void releaseDraw2MainScene(int _id) {
    SingleAPP.mainScene->defaultRelease(_id);
}

void releaseTrans2MainScene(int _id) {
    SingleAPP.mainScene->transparencyRelease(_id);
}

void releaseNorm2MainScene(int _id) {
    SingleAPP.mainScene->normalRelease(_id);
}

void releaseCapture2MainScene(int _boxId, int _captureId) {
    SingleAPP.mainScene->releaseAABB(_captureId, _boxId);
}
