package com.example.edgedetectionapp

import android.Manifest
import android.content.pm.PackageManager
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.util.Size
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.example.edgedetectionapp.databinding.ActivityMainBinding
import com.google.common.util.concurrent.ListenableFuture
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity() {

    private lateinit var imageAnalyzerExecutor: ExecutorService
    private lateinit var glRenderer: MyGLRenderer
    private lateinit var binding: ActivityMainBinding
    private lateinit var cameraProviderFuture: ListenableFuture<ProcessCameraProvider>

    private val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
    private val REQUEST_CODE_PERMISSIONS = 10

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        imageAnalyzerExecutor = Executors.newSingleThreadExecutor()


        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        glRenderer = MyGLRenderer()

        // 2. Configure and set the renderer for the GLSurfaceView
        binding.glSurfaceView.setEGLContextClientVersion(2) // Use OpenGL ES 2.0
        binding.glSurfaceView.setRenderer(glRenderer)
        binding.glSurfaceView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        if (allPermissionsGranted()) {
            startCamera()
        } else {
            ActivityCompat.requestPermissions(
                this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS
            )
        }
    }


    private fun allPermissionsGranted() =
        REQUIRED_PERMISSIONS.all {
            ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
        }


    private fun startCamera() {
        cameraProviderFuture = ProcessCameraProvider.getInstance(this)
        cameraProviderFuture.addListener({
            val cameraProvider = cameraProviderFuture.get()
            bindCameraUseCases(cameraProvider)
        }, ContextCompat.getMainExecutor(this))

    }


    private fun bindCameraUseCases(cameraProvider: ProcessCameraProvider) {

        val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
        val imageAnalysis = ImageAnalysis.Builder()
            .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_YUV_420_888)
            .setTargetResolution(Size(1280, 720))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build()
        imageAnalysis.setAnalyzer(imageAnalyzerExecutor, ImageAnalysis.Analyzer { imageProxy ->
            val yPlane = imageProxy.planes[0]
            val uPlane = imageProxy.planes[1]
            val vPlane = imageProxy.planes[2]
            val matPointer = NativeLib.processFrame(
                yBuffer = yPlane.buffer,
                uBuffer = uPlane.buffer,
                vBuffer = vPlane.buffer,
                width = imageProxy.width,
                height = imageProxy.height,
                pixelStrideY = yPlane.pixelStride,
                rowStrideY = yPlane.rowStride,
                pixelStrideUV = uPlane.pixelStride,
                rowStrideUV = uPlane.rowStride
            )
            Log.d("MainActivity", "Processed Mat Pointer: $matPointer")
            //Code for integration with openGL will be handled here for display
            glRenderer.updateFrame(matPointer)

            // 2. Request a redraw on   the GL thread
            binding.glSurfaceView.requestRender()
            imageProxy.close()
        })





        try {
            cameraProvider.unbindAll()
            cameraProvider.bindToLifecycle(
                this, cameraSelector,  imageAnalysis
            )

        } catch (exc: Exception) {
            Log.e("CameraX", "Binding failed", exc)
        }
    }


    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                startCamera()
            } else {
                finish()
            }
        }
    }




    companion object {
        init {
            System.loadLibrary("edgedetectionapp")
        }
    }
}
