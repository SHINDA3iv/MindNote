package com.example.mindnote.ui

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.example.mindnote.data.Workspace
import com.example.mindnote.data.WorkspaceRepository

class MainViewModel(application: Application) : AndroidViewModel(application) {
    private val repository = WorkspaceRepository.getInstance(application)
    private val _workspaces = MutableLiveData<List<Workspace>>()
    val workspaces: LiveData<List<Workspace>> = _workspaces
    private val expandedWorkspaces = mutableSetOf<String>()

    init {
        repository.workspaces.observeForever { workspaces ->
            _workspaces.value = workspaces
        }
    }

    fun createWorkspace(name: String, iconUri: String? = null) {
        repository.createWorkspace(name, iconUri)
    }

    fun updateWorkspace(workspace: Workspace) {
        repository.updateWorkspace(workspace)
    }

    fun getWorkspaceById(id: String): Workspace? {
        return repository.getWorkspaceById(id)
    }

    fun toggleWorkspaceExpansion(workspaceId: String, isExpanded: Boolean) {
        if (isExpanded) {
            expandedWorkspaces.add(workspaceId)
        } else {
            expandedWorkspaces.remove(workspaceId)
        }
        // Notify observers that the list has changed
        _workspaces.value = _workspaces.value
    }

    fun isWorkspaceExpanded(workspaceId: String): Boolean {
        return expandedWorkspaces.contains(workspaceId)
    }

    override fun onCleared() {
        super.onCleared()
        repository.workspaces.removeObserver { }
    }
} 