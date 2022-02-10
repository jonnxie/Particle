//
// Created by maybe on 2020/12/2.
//

#ifndef SHATTER_ENGINE_SHATTERUNIFORMBUFFER_H
#define SHATTER_ENGINE_SHATTERUNIFORMBUFFER_H

#include "shatterbuffer.h"
//#include "../Shatter_Model/shatter_model_include.h"

namespace shatter{
    namespace buffer{

        class ShatterUniformBuffer : public ShatterBuffer {
        public:
            static std::shared_ptr<ShatterUniformBuffer> createUniformBuffer(VkDevice* device,
                                                                             VkDeviceSize size);
            ShatterUniformBuffer();
            ~ShatterUniformBuffer() = default;
            bool initUniformBuffer(VkDeviceSize size);
        public:
            VkDeviceSize getUniformSize(){return ShatterBuffer::Get_Buffer_Size();};
        protected:
        };

    };
};




#endif //SHATTER_ENGINE_SHATTERUNIFORMBUFFER_H
