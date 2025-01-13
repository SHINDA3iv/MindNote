package com.example.studyproject

import android.app.Application
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class StudyProject : Application() {
    override fun onCreate() {
        super.onCreate()
    }
}
