package com.example.mindnote.data

import android.content.Context
import android.content.SharedPreferences

object UserManager {
    private const val PREFS_NAME = "user_prefs"
    private const val KEY_USER_NAME = "user_name"
    
    private lateinit var prefs: SharedPreferences
    
    fun init(context: Context) {
        prefs = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
    }
    
    fun getCurrentUserName(): String {
        return prefs.getString(KEY_USER_NAME, "Гость") ?: "Гость"
    }
    
    fun setCurrentUserName(userName: String) {
        prefs.edit().putString(KEY_USER_NAME, userName).apply()
    }
} 