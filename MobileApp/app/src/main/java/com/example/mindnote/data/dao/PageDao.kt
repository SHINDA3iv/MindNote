package com.example.mindnote.data.dao

import androidx.lifecycle.LiveData
import androidx.room.*
import com.example.mindnote.data.entity.Page

@Dao
interface PageDao {
    @Query("SELECT * FROM pages WHERE workspaceId = :workspaceId ORDER BY position ASC")
    fun getPagesForWorkspace(workspaceId: Long): LiveData<List<Page>>

    @Insert
    suspend fun insert(page: Page): Long

    @Update
    suspend fun update(page: Page)

    @Delete
    suspend fun delete(page: Page)

    @Query("SELECT * FROM pages WHERE id = :pageId")
    suspend fun getPageById(pageId: Long): Page?

    @Query("SELECT MAX(position) + 1 FROM pages WHERE workspaceId = :workspaceId")
    suspend fun getNextPosition(workspaceId: Long): Int?
} 