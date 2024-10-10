#include <memory>
#include <iostream>
#include <vk_sdk/Sdk.hpp>
#include <opencv2/opencv.hpp>

cv::Mat mat;
uint32_t height;
uint32_t width;

class ImageReceiver : public vkc::Receiver<vkc::Image> {
    // Override this method to handle messages.
    vkc::ReceiverStatus handle(const vkc::Message<vkc::Shared<vkc::Image>>& message) override {
        auto imageReader = message.payload.reader();
        long imageSize = imageReader.getData().size();

        height = imageReader.getHeight();
        width = imageReader.getWidth();
        auto encoding = imageReader.getEncoding();

        auto imageHeader = imageReader.getHeader();
        uint64_t stamp = imageHeader.getStampMonotonic() + imageHeader.getClockOffset();

        if(encoding ==  vkc::Image::Encoding::MONO8) {

            mat = cv::Mat(height, width, CV_8UC1, 
                        const_cast<unsigned char*>(imageReader.getData().asBytes().begin()));
            cv::cvtColor(mat, mat, cv::COLOR_GRAY2RGB);

        }else if (encoding ==  vkc::Image::Encoding::YUV420) {

            mat = cv::Mat(height* 3 / 2, width, CV_8UC1, 
                        const_cast<unsigned char*>(imageReader.getData().asBytes().begin()));
            cv::cvtColor(mat, mat, cv::COLOR_YUV2RGB_IYUV);

        }else if (encoding ==  vkc::Image::Encoding::BGR8) {
            mat = cv::Mat(height, width, CV_8UC3, 
                        const_cast<unsigned char*>(imageReader.getData().asBytes().begin()));
            // mat = mat.reshape(1, height);
            cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
        }else if (encoding ==  vkc::Image::Encoding::JPEG) {

            cv::Mat mat_jpeg(1, imageSize, CV_8UC1, 
                            const_cast<unsigned char*>(imageReader.getData().asBytes().begin()));
            mat = cv::imdecode(mat_jpeg, cv::IMREAD_COLOR);
            cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);

            assert(mat.rows == height);
            assert(mat.cols == width);

        }else
            throw std::runtime_error("unsupported encoding");
        

        return vkc::ReceiverStatus::Open;
    }
};


/*
image pixel definitions

(0,0)
|-----------|
|           |   
|           |
|-----------|(width,height)

Detections2d gives Xmin, Xmax, Ymin and Ymax which form the box for the detected image
These values are normalised and are required to be scaled according to its image's width and height
*/

class DetectionsReceiver : public vkc::Receiver<vkc::Detections2d> {
    // Override this method to handle messages.
    vkc::ReceiverStatus handle(const vkc::Message<vkc::Shared<vkc::Detections2d>>& message) override {
        auto all_detections = message.payload.reader();
        auto labels = all_detections.getLabels();
        int top_left_x, top_left_y, bottom_right_x, bottom_right_y;
        for (const auto& detection : all_detections.getDetections()){
            top_left_x = detection.getXmin() * width;
            top_left_y = detection.getYmin() * height;
            bottom_right_x = detection.getXmax()* width;
            bottom_right_y = detection.getYmax()* height;

            // /* comment out if you only want tag info
            //Open cv viewing
            cv::Point top_left( top_left_x, top_left_y);// scaling detections
            cv::Point bottom_right(bottom_right_x, bottom_right_y);// scaling detections
            cv::Scalar color(0, 255, 0);  // Green color in BGR
            int thickness = 2;  // Thickness of the rectangle border
            cv::rectangle(mat, top_left, bottom_right, color, thickness);

            // Define the text parameters
            std::string text = labels[detection.getLabelIdx()].cStr();;
            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.5;
            int fontThickness = 1;
            cv::Scalar textColor(0, 0, 255);
            // Put the text near the rectangle
            cv::putText(mat, text, cv::Point(top_left.x, top_left.y - 10), fontFace, fontScale, textColor, fontThickness);
            // */

            std::cout << text << " detected at (" 
                      << top_left_x << "," << top_left_y
                      << ") (" << bottom_right_x << "," << bottom_right_y << ")" 
                      << std::endl;
        }
        cv::Mat image = mat;
        //Uncomment to show the image in the created window
        cv::imshow("Display Window", image);
        cv::waitKey(1);
        
        return vkc::ReceiverStatus::Open;
    }
};

int main() {
    auto visualkit = vkc::VisualKit::create(std::nullopt);

    // Check that the object has been created successfully before proceeding.
    if (visualkit == nullptr) {
        std::cout << "Failed to create visualkit connection." << std::endl;
        return -1;
    }
    
    // Create the receiver that was defined by us.
    auto imageReceiver = std::make_unique<ImageReceiver>();
    auto detectionsReceiver = std::make_unique<DetectionsReceiver>();

    // Install the receiver into the data source so that the receiver can receive messages.
    visualkit->source().install("S1/camb", std::move(imageReceiver)); // edit source accordingly e.g. "S1/camd"
    visualkit->source().install("S1/camb/yolo", std::move(detectionsReceiver)); // edit source accordingly e.g. "S1/camd/yolo"

    // Start the data source so messages can be received by `myReceiver`.
    visualkit->source().start();

    // Destroy the window
    cv::destroyAllWindows();
    // Wait for the user to send the CTRL-C signal.
    vkc::waitForCtrlCSignal();

    // Stop the data source.
    //
    // You may pass `true` here if you want the soure to block until all data from it has been sent.
    // In this case, since data from visualkit is infinite, we do not want to block (as it will be forever).
    visualkit->source().stop(false);

    return 0;
}