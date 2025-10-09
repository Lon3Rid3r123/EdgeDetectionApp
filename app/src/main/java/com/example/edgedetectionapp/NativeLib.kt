package com.example.edgedetectionapp

object NativeLib {
    init {
        System.loadLibrary("edgedetectionapp")
    }

    // This function will be called for each camera frame
    external fun processFrame(
        yBuffer: java.nio.ByteBuffer,
        uBuffer: java.nio.ByteBuffer,
        vBuffer: java.nio.ByteBuffer,
        width: Int,
        height: Int,
        pixelStrideY: Int,
        rowStrideY: Int,
        pixelStrideUV: Int,
        rowStrideUV: Int
    ) : Long // Returns pointer to the processed Mat
}