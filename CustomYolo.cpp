#include "vk_sdk/capnp/Shared.hpp"
#include "vk_sdk/capnp/cameracontrol.capnp.h"
#include <iostream>
#include <memory>
#include <vk_sdk/Sdk.hpp>

/*
    yoloRequest
    0 -> manual mode. 
        once set to "0" the node stops forwarding image frames to the Neural Network. Each subsequent message sent that is "0" 
        forwards a frame to Neural Network

    1-16 -> auto mode.
        sends image frames every n frames.
*/

int main() {
    auto visualkit = vkc::VisualKit::create(std::nullopt);

    // Check that the object has been created successfully before proceeding.
    if (visualkit == nullptr) {
        std::cout << "Failed to create visualkit connection." << std::endl;
        return -1;
    }

    // Make a capnp object.
    auto builder = std::make_unique<capnp::MallocMessageBuilder>();
    auto yoloRequestBuilder = builder->initRoot<vkc::CameraControl>();

    std::string input;
    auto shared = vkc::Shared<vkc::CameraControl>(std::move(builder));

    // Obtain a receiver from the sink so that we can write to it.
    
    auto yoloRequestReceiver = visualkit->sink().obtain("S1/yolo_request", vkc::Type<vkc::CameraControl>()); //edit accordingly e.g. "S1/yolo_request"
    
    // Start the sink for it to start receiving messages.
    visualkit->sink().start();
    std::cout << "press q to exit, else send a frame to yolo detector after every n frames" <<std::endl;
    // Call the receiver of the sink to handle the message.
    while(input != "q"){
        std::cin >> input;
        std::cout << "passed: " << input << std::endl;
        yoloRequestBuilder.setYoloRequest(std::stoi(input)); // here we are converting the request from out input into an int. 
        yoloRequestReceiver->handle(shared);
    }
    // Stop the sink. 
    //
    // You may pass `true` here if you want the sink to block until all data from the source has been sent to it.
    //
    // See the documentation for `stop` for more information how to do this correctly.
    visualkit->sink().stop(false);

    return 0;
}
