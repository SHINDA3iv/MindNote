package com.example.mindnote.data.dao

import androidx.lifecycle.LiveData
import androidx.room.*
import com.example.mindnote.data.entity.Workspace

@Dao
interface WorkspaceDao {
    @Query("SELECT * FROM workspaces ORDER BY createdAt DESC")
    fun getAllWorkspaces(): LiveData<List<Workspace>>

    @Insert
    suspend fun insert(workspace: Workspace): Long

    @Update
    suspend fun update(workspace: Workspace)

    @Delete
    suspend fun delete(workspace: Workspace)

    @Query("SELECT * FROM workspaces WHERE id = :workspaceId")
    suspend fun getWorkspaceById(workspaceId: Long): Workspace?
} 