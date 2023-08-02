/**
origin:https://github.com/ppogg/YOLOv5-Lite

modify it for Balldi project

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**/

#include <iostream>
#include <string>
#include<ctime>

#include <MNN/MNNDefine.h>
#include <MNN/MNNForwardType.h>
#include <MNN/Interpreter.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>//balldi add

#include "Yolo.h"
#include "mnnyoloapi.h"


//    static const char* class_names[] = {
//        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
//        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
//        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
//        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
//        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
//        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
//        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
//        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
//        "hair drier", "toothbrush"
//    };


#define MNN_SUCC    0
#define MNN_ERR     1
#define MNN_NOBUF   2

static const int INPUT_SIZE = 640;
static const int net_size = 640;
static const int num_classes=80;
static const std::string model_name = "/home/pi/model_zoo/v5lite-s.mnn";
static std::vector<YoloLayerData> yolov5s_layers{
        {"937",    32, {{146, 217}, {231, 300}, {335, 433}}},
        {"917",    16, {{23,  29}, {43,  55},  {73,  105}}},
        {"output", 8,  {{4,  5}, {8,  10},  {13,  16}}},
};

//static std::shared_ptr<MNN::Interpreter> net;
static MNN::Interpreter * net;
static MNN::Session * session;
static MNN::Tensor * nhwc_Tensor;

std::vector<BoxInfo>
decode_infer(MNN::Tensor & data, int stride, const yolocv::YoloSize &frame_size, int net_size, int num_classes,
                     const std::vector<yolocv::YoloSize> &anchors, float threshold)
{
    std::vector<BoxInfo> result;
    int batchs, channels, height, width, pred_item ;
    batchs = data.shape()[0];
    channels = data.shape()[1];
    height = data.shape()[2];
    width = data.shape()[3];
    pred_item = data.shape()[4];

    auto data_ptr = data.host<float>();
    for(int bi=0; bi<batchs; bi++)
    {
        auto batch_ptr = data_ptr + bi*(channels*height*width*pred_item);
        for(int ci=0; ci<channels; ci++)
        {
            auto channel_ptr = batch_ptr + ci*(height*width*pred_item);
            for(int hi=0; hi<height; hi++)
            {
                auto height_ptr = channel_ptr + hi*(width * pred_item);
                for(int wi=0; wi<width; wi++)
                {
                    auto width_ptr = height_ptr + wi*pred_item;
                    auto cls_ptr = width_ptr + 5;

                    auto confidence = sigmoid(width_ptr[4]);

                    for(int cls_id=0; cls_id<num_classes; cls_id++)
                    {
                        float score = sigmoid(cls_ptr[cls_id]) * confidence;
                        if(score > threshold)
                        {
                            float cx = (sigmoid(width_ptr[0]) * 2.f - 0.5f + wi) * (float) stride;
                            float cy = (sigmoid(width_ptr[1]) * 2.f - 0.5f + hi) * (float) stride;
                            float w = pow(sigmoid(width_ptr[2]) * 2.f, 2) * anchors[ci].width;
                            float h = pow(sigmoid(width_ptr[3]) * 2.f, 2) * anchors[ci].height;

                            BoxInfo box;

                            box.x1 = std::max(0, std::min(frame_size.width, int((cx - w / 2.f) )));
                            box.y1 = std::max(0, std::min(frame_size.height, int((cy - h / 2.f) )));
                            box.x2 = std::max(0, std::min(frame_size.width, int((cx + w / 2.f) )));
                            box.y2 = std::max(0, std::min(frame_size.height, int((cy + h / 2.f) )));
                            box.score = score;
                            box.label = cls_id;
                            result.push_back(box);
                        }
                    }
                }
            }
        }
    }

    return result;
}


void nms(std::vector<BoxInfo> &input_boxes, float NMS_THRESH) {
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
                   * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        for (int j = i + 1; j < int(input_boxes.size());) {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            } else {
                j++;
            }
        }
    }
}


extern "C" {

//only use by cam_v4l2 , don't need lock
void init_mnn_yolo(void)
{
    if (nullptr == net) {
        std::cout<<"net is creating...."<<std::endl;
        //net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(model_name.c_str()));
        net = MNN::Interpreter::createFromFile(model_name.c_str());
    }

    if (nullptr == net) {
        std::cout<<"error:init net failed"<<std::endl;
        return ;
    }

    std::cout<<"info:create interpreter ok."<<std::endl;

    // config
    MNN::ScheduleConfig config;
    config.numThread = 3;
    config.type      = static_cast<MNNForwardType>(MNN_FORWARD_CPU);
    //config.type      = static_cast<MNNForwardType>(MNN_FORWARD_OPENGL);       //H2的GPU OPENGL版本太低，不支持MNN GPU加速
    MNN::BackendConfig backendConfig;
    backendConfig.memory = (MNN::BackendConfig::MemoryMode)2;
    backendConfig.precision = (MNN::BackendConfig::PrecisionMode)2;
    // backendConfig.precision =  MNN::PrecisionMode Precision_Normal; // static_cast<PrecisionMode>(Precision_Normal);
    config.backendConfig = &backendConfig;

    std::cout<<"session is creating...."<<std::endl;
    session = net->createSession(config);

    if (nullptr == net) {
        std::cout<<"error:create session failed"<<std::endl;
        return ;
    }

    std::cout<<"info:create session ok."<<std::endl;

    std::vector<int> dims{1, INPUT_SIZE, INPUT_SIZE, 3};
    nhwc_Tensor = MNN::Tensor::create<float>(dims, NULL, MNN::Tensor::TENSORFLOW);
}

void close_mnn_yolo(void)
{
    //net->releaseModel();
    //net->releaseSession(session);
    delete net;
}

/**
 * @param arr_rst  ptr to recv result
 * @param rst_len  rst struct data num
 * @param jpg_raw  jpg raw data
 * @param rst_len  num of jpg raw bytes
 */
void get_result_by_jpg_raw(t_result *arr_rst, int *rst_len, unsigned char *jpg_raw, int jpg_len)
{
    if(NULL == jpg_raw || 0 == jpg_len){
        std::cout << "err para in" << std::endl;
        return ;
    }

    std::vector<unsigned char> buff(jpg_raw, jpg_raw + jpg_len);
    cv::Mat mat = cv::imdecode(buff, CV_LOAD_IMAGE_COLOR);
    cv::Mat image;
    cv::resize(mat, image, cv::Size(INPUT_SIZE, INPUT_SIZE));

    image.convertTo(image, CV_32FC3);
    // image = (image * 2 / 255.0f) - 1;
    image = image /255.0f;

    // wrapping input tensor, convert nhwc to nchw
    auto nhwc_data   = nhwc_Tensor->host<float>();
    auto nhwc_size   = nhwc_Tensor->size();
    std::memcpy(nhwc_data, image.data, nhwc_size);


    auto inputTensor = net->getSessionInput(session, nullptr);
    inputTensor->copyFromHostTensor(nhwc_Tensor);

    // run network
    clock_t startTime,endTime;
    startTime = clock();//计时开始
    net->runSession(session);
    endTime = clock();//计时结束
    cout << "use time: " <<(double)(endTime - startTime) / 1000.0 << "ms" << endl;

    // get output data
    std::string output_tensor_name0 = yolov5s_layers[2].name ;
    std::string output_tensor_name1 = yolov5s_layers[1].name ;
    std::string output_tensor_name2 = yolov5s_layers[0].name ;

    MNN::Tensor *tensor_scores  = net->getSessionOutput(session, output_tensor_name0.c_str());
    MNN::Tensor *tensor_boxes   = net->getSessionOutput(session, output_tensor_name1.c_str());
    MNN::Tensor *tensor_anchors = net->getSessionOutput(session, output_tensor_name2.c_str());

    MNN::Tensor tensor_scores_host(tensor_scores, tensor_scores->getDimensionType());
    MNN::Tensor tensor_boxes_host(tensor_boxes, tensor_boxes->getDimensionType());
    MNN::Tensor tensor_anchors_host(tensor_anchors, tensor_anchors->getDimensionType());

    tensor_scores->copyToHostTensor(&tensor_scores_host);
    tensor_boxes->copyToHostTensor(&tensor_boxes_host);
    tensor_anchors->copyToHostTensor(&tensor_anchors_host);

    std::vector<BoxInfo> result;
    std::vector<BoxInfo> boxes;

    yolocv::YoloSize yolosize = yolocv::YoloSize{INPUT_SIZE,INPUT_SIZE};

    float threshold = 0.25;
    float nms_threshold = 0.3;

    boxes = decode_infer(tensor_scores_host, yolov5s_layers[2].stride,  yolosize, net_size, num_classes, yolov5s_layers[2].anchors, threshold);
    result.insert(result.begin(), boxes.begin(), boxes.end());

    boxes = decode_infer(tensor_boxes_host, yolov5s_layers[1].stride,  yolosize, net_size, num_classes, yolov5s_layers[1].anchors, threshold);
    result.insert(result.begin(), boxes.begin(), boxes.end());

    boxes = decode_infer(tensor_anchors_host, yolov5s_layers[0].stride,  yolosize, net_size, num_classes, yolov5s_layers[0].anchors, threshold);
    result.insert(result.begin(), boxes.begin(), boxes.end());

    nms(result, nms_threshold);

    *rst_len = result.size();
    //switch result
    vector<BoxInfo>::iterator itor = result.begin();
    t_result* rst_tmp = arr_rst;

    for(; itor != result.end(); ++itor)
    {
        rst_tmp->x1 = (*itor).x1;
        rst_tmp->y1 = (*itor).y1;
        rst_tmp->x2 = (*itor).x2;
        rst_tmp->y2 = (*itor).y2;
        rst_tmp->label = (*itor).label;
        rst_tmp->id = (*itor).id;
        rst_tmp->score = (*itor).score;
        ++rst_tmp;
    }
}

}   //extern "C"
