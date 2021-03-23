package com.cci.opengl3nativerender

import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val openGLRenderer = GlTriangleRenderer()
        // Example of a call to a native method
        val surfaceView = findViewById<SurfaceView>(R.id.surface_view)
        surfaceView.holder.addCallback(object : SurfaceHolder.Callback2{
            override fun surfaceCreated(holder: SurfaceHolder) {
                openGLRenderer.drawTriangle(holder.surface)
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
            }

            override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
            }
        })
    }
}