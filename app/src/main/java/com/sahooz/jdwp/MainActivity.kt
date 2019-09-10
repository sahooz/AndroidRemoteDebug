package com.sahooz.jdwp

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Thread{ replaceDebug(8000) }.start()
        btnLog.setOnClickListener {
            Log.d("JDWP", "Log...............")
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        end()
    }

    external fun replaceDebug(port: Int)
    external fun end()

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}
