//
// Created by jonnxie on 2022/3/21.
//

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <vector>

class scene{
public:
    std::vector<int> compute_vec;
    std::vector<int> offscreen_vec;
    std::vector<int> default_vec;
    std::vector<int> normal_vec;
    std::vector<int> transparency_vec;

};


#endif //MAIN_SCENE_H
