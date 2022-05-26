//
// Created by jonnxie on 2022/5/18.
//

#ifndef MAIN_OPERATOR_H
#define MAIN_OPERATOR_H

#include "Engine/Item/shatter_macro.h"
#include <mutex>
#include <string>
#include <algorithm>
#include <memory>

class Object;

void pushObject2MainScene(int _id, SP(Object) _obj);
void releaseObject2MainScene(int _id, SP(Object) _obj);

void pushDraw2MainScene(int _id);
void releaseDraw2MainScene(int _id);

void pushTrans2MainScene(int _id);
void releaseTrans2MainScene(int _id);

void pushNorm2MainScene(int _id);
void releaseNorm2MainScene(int _id);

void pushCapture2MainScene(int _boxId, int _captureId);
void releaseCapture2MainScene(int _boxId, int _captureId);




#endif //MAIN_OPERATOR_H
