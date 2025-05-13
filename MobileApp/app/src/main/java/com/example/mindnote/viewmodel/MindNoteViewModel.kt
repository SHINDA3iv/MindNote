package com.example.mindnote.viewmodel

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.viewModelScope
import com.example.mindnote.data.AppDatabase
import com.example.mindnote.data.entity.Page
import com.example.mindnote.data.entity.PageElement
import com.example.mindnote.data.entity.Workspace
import com.example.mindnote.repository.MindNoteRepository
import kotlinx.coroutines.launch

class MindNoteViewModel(application: Application) : AndroidViewModel(application) {
    private val repository: MindNoteRepository
    val allWorkspaces: LiveData<List<Workspace>>

    init {
        val database = AppDatabase.getDatabase(application)
        repository = MindNoteRepository(
            database.workspaceDao(),
            database.pageDao(),
            database.pageElementDao()
        )
        allWorkspaces = repository.getAllWorkspaces()
    }

    // Workspace operations
    fun createWorkspace(name: String) = viewModelScope.launch {
        repository.createWorkspace(name)
    }

    fun updateWorkspace(workspace: Workspace) = viewModelScope.launch {
        repository.updateWorkspace(workspace)
    }

    fun deleteWorkspace(workspace: Workspace) = viewModelScope.launch {
        repository.deleteWorkspace(workspace)
    }

    // Page operations
    fun getPagesForWorkspace(workspaceId: Long): LiveData<List<Page>> =
        repository.getPagesForWorkspace(workspaceId)

    fun createPage(workspaceId: Long, title: String) = viewModelScope.launch {
        repository.createPage(workspaceId, title)
    }

    fun updatePage(page: Page) = viewModelScope.launch {
        repository.updatePage(page)
    }

    fun deletePage(page: Page) = viewModelScope.launch {
        repository.deletePage(page)
    }

    // Page element operations
    fun getElementsForPage(pageId: Long): LiveData<List<PageElement>> =
        repository.getElementsForPage(pageId)

    fun createElement(element: PageElement) = viewModelScope.launch {
        repository.createElement(element)
    }

    fun updateElement(element: PageElement) = viewModelScope.launch {
        repository.updateElement(element)
    }

    fun deleteElement(element: PageElement) = viewModelScope.launch {
        repository.deleteElement(element)
    }
} 