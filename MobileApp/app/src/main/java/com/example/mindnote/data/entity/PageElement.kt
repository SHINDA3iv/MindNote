package com.example.mindnote.data.entity

import androidx.room.Entity
import androidx.room.ForeignKey
import androidx.room.PrimaryKey

@Entity(
    tableName = "page_elements",
    foreignKeys = [
        ForeignKey(
            entity = Page::class,
            parentColumns = ["id"],
            childColumns = ["pageId"],
            onDelete = ForeignKey.CASCADE
        )
    ]
)
data class PageElement(
    @PrimaryKey(autoGenerate = true)
    val id: Long = 0,
    var pageId: Long,
    var type: ElementType,
    var content: String,
    var x: Int,
    var y: Int,
    var width: Int,
    var height: Int,
    var createdAt: Long = System.currentTimeMillis()
)

enum class ElementType {
    TEXT,
    CHECKBOX,
    IMAGE,
    VIDEO,
    FILE,
    PAGE_LINK
} 