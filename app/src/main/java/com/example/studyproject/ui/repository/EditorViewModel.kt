package com.example.studyproject.ui.repository

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.example.studyproject.data.model.BaseElement
import com.example.studyproject.data.model.Workspace
import com.example.studyproject.data.repository.WorkspaceRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject

@HiltViewModel
class EditorViewModel @Inject constructor(
    private val repository: WorkspaceRepository
) : ViewModel() {

    private val _workspace = MutableLiveData<Workspace>()
    val workspace: LiveData<Workspace> = _workspace

    fun loadWorkspace(id: String) {
        _workspace.value = repository.getWorkspace(id)
    }

    fun addElement(element: BaseElement) {
        _workspace.value?.let { workspace ->
            workspace.elements.add(element)
            repository.updateWorkspace(workspace.id, workspace)
            _workspace.value = workspace
        }
    }
}
