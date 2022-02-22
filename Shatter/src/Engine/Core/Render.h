//
// Created by AnWell on 2022/2/20.
//

#ifndef MAIN_RENDER_H
#define MAIN_RENDER_H

#include "precompiledhead.h"
#include "Engine/Item/shatter_macro.h"

class Render {
public:
    Render() = default;
    ~Render() = default;

    DefineUnCopy(Render);
public:
    virtual void init();

    virtual void update();

    virtual void cleanup();

    virtual void cleanupObject();

public:
    SyncContainerOperation(offscreen);
    SyncContainerOperation(default);
    SyncContainerOperation(trans);
    SyncContainerOperation(normal);
    SyncContainerOperation(compute);

public:
    std::vector<uint32_t> m_offscreen_id_vec;
    bool m_offscreen_changed = false;

    std::vector<uint32_t>  m_default_id_vec;
    bool m_default_changed = false;

    std::vector<uint32_t>  m_trans_id_vec;
    bool m_trans_changed = false;

    std::vector<uint32_t>  m_normal_id_vec;//Dont output g attachment
    bool m_normal_changed = false;

    std::vector<uint32_t>  m_compute_id_vec;
    bool m_compute_changed = false;
    void test(){
//        m_compute_id_vec[0].clear();
    }
    bool guiChanged = false;
    bool windowStill = true;
};


#endif //MAIN_RENDER_H
