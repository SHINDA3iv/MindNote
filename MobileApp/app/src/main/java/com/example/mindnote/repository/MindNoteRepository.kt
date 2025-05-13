package com.example.mindnote.repository

import androidx.lifecycle.LiveData
import com.example.mindnote.data.dao.PageDao
import com.example.mindnote.data.dao.PageElementDao
import com.example.mindnote.data.dao.WorkspaceDao
import com.example.mindnote.data.entity.Page
import com.example.mindnote.data.entity.PageElement
import com.example.mindnote.data.entity.Workspace

class MindNoteRepository(
    private val workspaceDao: WorkspaceDao,
    private val pageDao: PageDao,
    private val pageElementDao: PageElementDao
) {
    // Workspace operations
    fun getAllWorkspaces(): LiveData<List<Workspace>> = workspaceDao.getAllWorkspaces()
    
    suspend fun createWorkspace(name: String): Long {
        return workspaceDao.insert(Workspace(name = name))
    }

    suspend fun updateWorkspace(workspace: Workspace) = workspaceDao.update(workspace)
    
    suspend fun deleteWorkspace(workspace: Workspace) = workspaceDao.delete(workspace)

    // Page operations
    fun getPagesForWorkspace(workspaceId: Long): LiveData<List<Page>> = 
        pageDao.getPagesForWorkspace(workspaceId)

    suspend fun createPage(workspaceId: Long, title: String): Long {
        val position = pageDao.getNextPosition(workspaceId) ?: 0
        return pageDao.insert(Page(workspaceId = workspaceId, title = title, position = position))
    }

    suspend fun updatePage(page: Page) = pageDao.update(page)
    
    suspend fun deletePage(page: Page) = pageDao.delete(page)

    // Page element operations
    fun getElementsForPage(pageId: Long): LiveData<List<PageElement>> = 
        pageElementDao.getElementsForPage(pageId)

    suspend fun createElement(element: PageElement): Long = pageElementDao.insert(element)
    
    suspend fun updateElement(element: PageElement) = pageElementDao.update(element)
    
    suspend fun deleteElement(element: PageElement) = pageElementDao.delete(element)
} 