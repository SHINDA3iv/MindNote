package com.example.mindnote

import android.content.Context
import android.net.Uri
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.example.mindnote.data.ContentItem
import com.example.mindnote.data.Workspace
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import java.io.File

class MainViewModel : ViewModel() {
    private val _workspaces = MutableLiveData<List<Workspace>>()
    val workspaces: LiveData<List<Workspace>> = _workspaces

    private val _currentWorkspace = MutableLiveData<Workspace>()
    val currentWorkspace: LiveData<Workspace> = _currentWorkspace

    private val gson = Gson()

    fun loadWorkspaces(context: Context) {
        val file = File(context.filesDir, "workspaces.json")
        if (file.exists()) {
            val json = file.readText()
            val type = object : TypeToken<List<Workspace>>() {}.type
            val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type)
            _workspaces.value = loadedWorkspaces
        } else {
            _workspaces.value = emptyList()
        }
    }

    fun saveWorkspaces(context: Context) {
        val file = File(context.filesDir, "workspaces.json")
        val json = gson.toJson(_workspaces.value)
        file.writeText(json)
    }

    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        val workspace = Workspace(name, iconUri)
        val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
        currentList.add(workspace)
        _workspaces.value = currentList
        return workspace
    }

    fun updateWorkspace(workspace: Workspace) {
        val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
        val index = currentList.indexOfFirst { it.id == workspace.id }
        if (index != -1) {
            currentList[index] = workspace
            _workspaces.value = currentList
        }
    }

    fun setCurrentWorkspace(workspace: Workspace) {
        workspace.lastAccessed = System.currentTimeMillis()
        updateWorkspace(workspace)
        _currentWorkspace.value = workspace
    }

    fun toggleFavorite(workspace: Workspace) {
        workspace.isFavorite = !workspace.isFavorite
        updateWorkspace(workspace)
    }

    fun getFavorites(): List<Workspace> {
        return _workspaces.value?.filter { it.isFavorite } ?: emptyList()
    }

    fun getRecentlyAccessed(limit: Int = 5): List<Workspace> {
        return _workspaces.value?.sortedByDescending { it.lastAccessed }?.take(limit) ?: emptyList()
    }

    fun addContentItem(workspace: Workspace, item: ContentItem) {
        workspace.items.add(item)
        updateWorkspace(workspace)
        if (_currentWorkspace.value?.id == workspace.id) {
            _currentWorkspace.value = workspace
        }
    }

    fun removeContentItem(workspace: Workspace, itemId: String) {
        workspace.items.removeAll { item ->
            when (item) {
                is ContentItem.TextItem -> item.id == itemId
                is ContentItem.CheckboxItem -> item.id == itemId
                is ContentItem.NumberedListItem -> item.id == itemId
                is ContentItem.BulletListItem -> item.id == itemId
                is ContentItem.ImageItem -> item.id == itemId
                is ContentItem.FileItem -> item.id == itemId
            }
        }
        updateWorkspace(workspace)
        if (_currentWorkspace.value?.id == workspace.id) {
            _currentWorkspace.value = workspace
        }
    }

    fun updateContentItem(workspace: Workspace, item: ContentItem) {
        val index = workspace.items.indexOfFirst { existingItem ->
            when (existingItem) {
                is ContentItem.TextItem -> existingItem.id == when (item) {
                    is ContentItem.TextItem -> item.id
                    else -> ""
                }
                is ContentItem.CheckboxItem -> existingItem.id == when (item) {
                    is ContentItem.CheckboxItem -> item.id
                    else -> ""
                }
                is ContentItem.NumberedListItem -> existingItem.id == when (item) {
                    is ContentItem.NumberedListItem -> item.id
                    else -> ""
                }
                is ContentItem.BulletListItem -> existingItem.id == when (item) {
                    is ContentItem.BulletListItem -> item.id
                    else -> ""
                }
                is ContentItem.ImageItem -> existingItem.id == when (item) {
                    is ContentItem.ImageItem -> item.id
                    else -> ""
                }
                is ContentItem.FileItem -> existingItem.id == when (item) {
                    is ContentItem.FileItem -> item.id
                    else -> ""
                }
            }
        }
        if (index != -1) {
            workspace.items[index] = item
            updateWorkspace(workspace)
            if (_currentWorkspace.value?.id == workspace.id) {
                _currentWorkspace.value = workspace
            }
        }
    }
}
