package com.example.studyproject.data.repository

import com.example.studyproject.data.model.Workspace

class WorkspaceRepository {
    private val workspaces = mutableMapOf<String, Workspace>()

    fun getWorkspace(id: String): Workspace? = workspaces[id]

    fun addWorkspace(workspace: Workspace) {
        workspaces[workspace.id] = workspace
    }

    fun updateWorkspace(id: String, workspace: Workspace) {
        workspaces[id] = workspace
    }

    fun getAllWorkspaces(): List<Workspace> = workspaces.values.toList()
}