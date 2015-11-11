# Author: Jimmy Baraglia
# YARP needs CMake 2.8.7 or greater

cmake_minimum_required(VERSION 2.8.7)
# find YARP
find_package(YARP REQUIRED)
find_package(OpenCV REQUIRED )
find_package(ICUB)
# add YARP include directories
include_directories(${YARP_INCLUDE_DIRS})

# Recognition
add_executable(Perception 
Perception/main.cpp 
Perception/Perception.h Perception/Recognition.cpp 
Perception/trackingC.h Perception/trackingC.cpp
)
# link with YARP libraries and Opencv Librariesr
target_link_libraries(Perception ${YARP_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(Perception icubmod)


# Action
add_executable(Action 
Action/main.cpp 
Action/Action.h Action/Action.cpp
)
# link with YARP libraries
target_link_libraries(Action ${YARP_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(Action icubmod ctrlLib)

# Task space prediction
add_executable(Prediction 
Task_space_prediction/main.cpp 
Task_space_prediction/BNET/src/dbn.h Task_space_prediction/BNET/src/dbn.cpp

Task_space_prediction/BNET/src/BNet.h Task_space_prediction/BNET/src/BNet.cpp
Task_space_prediction/BNET/src/BNode.h Task_space_prediction/BNet/src/BNode.cpp
Task_space_prediction/BNET/src/BEdge.h Task_space_prediction/BNET/src/BEdge.cpp
Task_space_prediction/BNET/src/BPVector.h Task_space_prediction/BNET/src/BPVector.cpp
Task_space_prediction/BNET/src/displayNodes.h Task_space_prediction/BNET/src/displayNodes.cpp 
)
# link with YARP libraries
target_link_libraries(Prediction ${YARP_LIBRARIES} ${OpenCV_LIBS})


install(TARGETS Perception DESTINATION bin)
install(TARGETS Action DESTINATION bin)
install(TARGETS Prediction DESTINATION bin)

