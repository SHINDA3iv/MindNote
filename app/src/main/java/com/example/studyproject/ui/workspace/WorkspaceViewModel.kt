package com.example.studyproject.ui.workspace

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.example.studyproject.data.model.Workspace
import com.example.studyproject.data.repository.WorkspaceRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import java.util.UUID
import javax.inject.Inject

@HiltViewModel
class WorkspaceViewModel @Inject constructor(
    private val repository: WorkspaceRepository
) : ViewModel() {

    private val _workspaces = MutableLiveData<List<Workspace>>()
    val workspaces: LiveData<List<Workspace>> = _workspaces

    init {
        loadWorkspaces()
    }

    private fun loadWorkspaces() {
        _workspaces.value = repository.getAllWorkspaces()
    }

    fun createWorkspace(name: String) {
        val newWorkspace = Workspace(UUID.randomUUID().toString(), name)
        repository.addWorkspace(newWorkspace)
        loadWorkspaces()
    }
}