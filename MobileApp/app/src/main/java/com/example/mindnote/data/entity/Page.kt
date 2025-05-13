package com.example.mindnote.data.entity

import androidx.room.Entity
import androidx.room.ForeignKey
import androidx.room.PrimaryKey

@Entity(
    tableName = "pages",
    foreignKeys = [
        ForeignKey(
            entity = Workspace::class,
            parentColumns = ["id"],
            childColumns = ["workspaceId"],
            onDelete = ForeignKey.CASCADE
        )
    ]
)
data class Page(
    @PrimaryKey(autoGenerate = true)
    val id: Long = 0,
    var workspaceId: Long,
    var title: String,
    var position: Int,
    var createdAt: Long = System.currentTimeMillis()
) 