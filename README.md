Built a minimal Android app that captures frames, processes them using OpenCV in C++(via JNI), and displays the processed output using OpenGL ES.
Additionally, created a small TypeScript-based webpage that can receive a dummy processed frame(base64) and display it.

Key Features
1. Camera Feed Integration (Android)
   Using CameraX to capture and analyse frames.
2. Frame Processing via OpenCV (C++)
   Sends each frame to native code using JNI.
   Applies Canny Edge Detection using OpenCV (C++).
   Returns processed frames
3. Render Output with OpenGL ES
   Renders the processed image using OpenGL ES 2.0(as a texture)
4. Web Viewer(TypeScript)
   Minimal Webpage that displays a static frame

<img width="341" height="474" alt="image" src="https://github.com/user-attachments/assets/559d25ad-9056-4860-9118-a7b0e9bfdc3d" />


⚙️ Setup Instructions


1. Android NDK and CMake
This project uses C++ for performance-critical image processing, which requires the Android NDK (Native Development Kit) and CMake to build the native code.

Open the project in Android Studio.

Navigate to File > Settings > Appearance & Behavior > System Settings > Android SDK.

Click on the SDK Tools tab.

Ensure that NDK (Side by side) and CMake are checked and installed.

2. OpenCV Android SDK Dependency
The project is pre-configured to link against the OpenCV (Open Source Computer Vision) library for Android.

Download: Go to the official OpenCV releases page and download the latest OpenCV Android pack (e.g., opencv-4.x.x-android-sdk.zip).

Unzip: Extract the contents of the ZIP file.

Place: Move the extracted OpenCV-android-sdk folder to a simple path on your computer that does not contain spaces. The project is configured to find it at this specific location:

C:\dev\OpenCV-android-sdk
Verify Path (Optional): If you place the SDK in a different location, you must update the path in the app/src/main/cpp/CMakeLists.txt file by changing the following line:

CMake

set(OpenCV_DIR C:/your/custom/path/OpenCV-android-sdk/sdk/native/jni)

🏛️ Project Architecture


This project integrates several key technologies to create a real-time video processing pipeline. The architecture is split into two main parts: the native Android application and a simple web-based viewer.

1. The JNI Bridge: Connecting Kotlin and C++
The core of this application is the Java Native Interface (JNI), which acts as a bridge between the Android front-end (written in Kotlin) and the high-performance back-end (written in C++).

Kotlin (Android): Manages the user interface, camera access (using CameraX), and the OpenGL rendering surface (GLSurfaceView). It is responsible for capturing raw camera frames.

C++ (Native): Contains all the heavy-lifting logic for image processing with OpenCV and rendering with OpenGL ES.

The JNI allows us to define external functions in Kotlin that are implemented in C++. This way, we can pass raw image data from the camera directly to our native C++ code for efficient processing, bypassing the slower Android framework for these intensive tasks.

2. Frame Processing Flow
A single camera frame goes through the following pipeline to get from the camera to the screen:

Capture (Kotlin): The CameraX library captures a frame from the device's camera. The frame is provided in a raw YUV_420_888 format.

JNI Call (Kotlin → C++): The raw YUV data (as ByteBuffer objects) is passed to the native processFrame function via the JNI bridge.

OpenCV Processing (C++):

The raw YUV data is converted into an OpenCV cv::Mat object. For simplicity, only the Y plane (grayscale data) is used.

The Canny edge detection algorithm is applied to the grayscale Mat to find edges.

The resulting single-channel (black and white) edge Mat is converted to a four-channel RGBA Mat, which is the format required by OpenGL for rendering.

Return Pointer (C++ → Kotlin): The C++ function returns a memory address (long pointer) pointing to the final RGBA Mat object.

OpenGL Rendering (Kotlin → C++):

The MainActivity receives the pointer and triggers a redraw on the GLSurfaceView.

The renderer calls the native drawFrame function, passing the pointer back across the JNI bridge.

The C++ drawFrame function uses the pointer to access the RGBA Mat data and uploads it to a GPU texture using glTexImage2D.

Finally, OpenGL ES 2.0 draws this texture onto a simple rectangle that fills the screen.

This entire flow is repeated for every frame, creating the illusion of a live, processed video feed.

3. TypeScript Web Viewer
The web viewer is a separate, minimal component designed to demonstrate the ability to display processed results in a web environment.

Technology: It's a simple, static webpage built with HTML and TypeScript.

Functionality:

It displays a static, pre-processed sample frame. This frame is saved from a live run of the Android app and embedded directly into the code as a Base64 string.

The TypeScript code retrieves the HTML elements and sets the image source from the Base64 data.

It overlays a simple text block on top of the image to display mock frame statistics like FPS and resolution.

Build Process: The TypeScript code (.ts) is compiled into standard JavaScript (.js) using the tsc compiler, which is then included in the index.html file.
