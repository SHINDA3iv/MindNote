package com.example.mindnote.data.dao

import androidx.lifecycle.LiveData
import androidx.room.*
import com.example.mindnote.data.entity.PageElement

@Dao
interface PageElementDao {
    @Query("SELECT * FROM page_elements WHERE pageId = :pageId ORDER BY createdAt ASC")
    fun getElementsForPage(pageId: Long): LiveData<List<PageElement>>

    @Insert
    suspend fun insert(element: PageElement): Long

    @Update
    suspend fun update(element: PageElement)

    @Delete
    suspend fun delete(element: PageElement)

    @Query("SELECT * FROM page_elements WHERE id = :elementId")
    suspend fun getElementById(elementId: Long): PageElement?
} 