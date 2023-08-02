/**
origin:https://github.com/ppogg/YOLOv5-Lite

modify it for Balldi project

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**/
#ifndef Yolo_H
#define Yolo_H

#include <MNN/MNNDefine.h>
#include <MNN/MNNForwardType.h>
#include <MNN/Interpreter.hpp>

#include "util.h"

namespace yolocv {
    typedef struct {
        int width;
        int height;
    } YoloSize;
}

typedef struct {
    std::string name;
    int stride;
    std::vector<yolocv::YoloSize> anchors;
} YoloLayerData;

class BoxInfo
{
public:
    int x1,y1,x2,y2,label,id;
    float score;
};

 std::vector<BoxInfo>
    decode_infer(MNN::Tensor & data, int stride, const yolocv::YoloSize &frame_size, int net_size, int num_classes,
                 const std::vector<yolocv::YoloSize> &anchors, float threshold);

     void nms(std::vector<BoxInfo> &result, float nms_threshold);



#endif //Yolo_H
