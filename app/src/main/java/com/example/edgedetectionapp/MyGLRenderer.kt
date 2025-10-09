

package com.example.edgedetectionapp

import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MyGLRenderer : GLSurfaceView.Renderer {


    private var matPointer: Long = 0


    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {

        NativeLib.onSurfaceCreated()
    }


    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {

        NativeLib.onSurfaceChanged(width, height)
    }


    override fun onDrawFrame(gl: GL10?) {

        NativeLib.drawFrame(matPointer)
    }


    fun updateFrame(matPointer: Long) {
        this.matPointer = matPointer
    }
}

