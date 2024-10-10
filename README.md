# Request-based-yolo-example

## Compiling 
```
mkdir build && cd build
cmake ..
make
```
## Running 
On one terminal, run 
```
./yolo_request 
```
this allows the user to set the frame rate or request frames to be forwarded to the Image detector

On the other terminal, run 
```
./image_yolo_viewer 
```
This shows the user the location of the detections as well as its labels.

At the moment it prints out the image and draws the box for the detections along with the label. Comment out the opencv viewing part if the user wishes to only obtain detection locations.

