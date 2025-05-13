package com.example.mindnote.data.entity

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity(tableName = "workspaces")
data class Workspace(
    @PrimaryKey(autoGenerate = true)
    val id: Long = 0,
    var name: String,
    var createdAt: Long = System.currentTimeMillis()
) 