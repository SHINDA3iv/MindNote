package com.example.mindnote

import android.content.Context
import android.net.Uri
import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.example.mindnote.data.ContentItem
import com.example.mindnote.data.Workspace
import com.example.mindnote.data.WorkspaceRepository
import com.google.gson.Gson
import com.google.gson.GsonBuilder
import com.google.gson.JsonDeserializationContext
import com.google.gson.JsonDeserializer
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.google.gson.JsonPrimitive
import com.google.gson.JsonSerializationContext
import com.google.gson.JsonSerializer
import com.google.gson.reflect.TypeToken
import java.io.File
import java.lang.reflect.Type

class MainViewModel : ViewModel() {
    private lateinit var repository: WorkspaceRepository
    private val _currentWorkspace = MutableLiveData<Workspace>()
    
    // Текущее открытое рабочее пространство
    val currentWorkspace: LiveData<Workspace> = _currentWorkspace
    
    // Инициализация репозитория, должна быть вызвана перед использованием
    fun init(context: Context) {
        Log.d("StartAppTag", "Initializing MainViewModel")
        try {
            repository = WorkspaceRepository.getInstance(context)
            Log.d("StartAppTag", "MainViewModel initialized successfully")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error initializing MainViewModel", e)
            throw e
        }
    }
    
    // LiveData со списком всех рабочих пространств
    val workspaces: LiveData<List<Workspace>> 
        get() = repository.workspaces
    
    // Сохранение всех рабочих пространств
    fun saveWorkspaces() {
        Log.d("StartAppTag", "Saving workspaces")
        try {
            repository.saveWorkspaces()
            Log.d("StartAppTag", "Workspaces saved successfully")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error saving workspaces", e)
            throw e
        }
    }
    
    // Обновление рабочих пространств из хранилища
    fun loadWorkspaces() {
        Log.d("StartAppTag", "Loading workspaces")
        try {
            repository.loadWorkspaces()
            Log.d("StartAppTag", "Workspaces loaded successfully")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error loading workspaces", e)
            throw e
        }
    }
    
    // Создание нового рабочего пространства
    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        Log.d("StartAppTag", "Creating new workspace: $name")
        try {
            val workspace = repository.createWorkspace(name, iconUri)
            Log.d("StartAppTag", "Workspace created successfully: $name")
            return workspace
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error creating workspace: $name", e)
            throw e
        }
    }
    
    // Обновление существующего рабочего пространства
    fun updateWorkspace(workspace: Workspace) {
        Log.d("StartAppTag", "Updating workspace: ${workspace.name}")
        try {
            repository.updateWorkspace(workspace)
            Log.d("StartAppTag", "Workspace updated successfully: ${workspace.name}")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error updating workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Установка текущего рабочего пространства
    fun setCurrentWorkspace(workspace: Workspace) {
        Log.d("StartAppTag", "Setting current workspace to: ${workspace.name}")
        try {
            workspace.lastAccessed = System.currentTimeMillis()
            repository.updateWorkspace(workspace)
            _currentWorkspace.value = workspace
            Log.d("StartAppTag", "Current workspace set successfully: ${workspace.name} with ${workspace.items.size} items")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error setting current workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Получение рабочего пространства по имени
    fun getWorkspaceByName(name: String): Workspace? {
        Log.d("StartAppTag", "Getting workspace by name: $name")
        return try {
            val workspace = repository.getWorkspaceByName(name)
            Log.d("StartAppTag", "Workspace found: $name")
            workspace
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error getting workspace by name: $name", e)
            null
        }
    }
    
    // Добавление нового элемента содержимого в рабочее пространство
    fun addContentItem(workspace: Workspace, item: ContentItem) {
        Log.d("StartAppTag", "Adding content item to workspace: ${workspace.name}")
        try {
            repository.addContentItem(workspace, item)
            if (_currentWorkspace.value?.id == workspace.id) {
                _currentWorkspace.value = repository.getWorkspaceById(workspace.id)
            }
            Log.d("StartAppTag", "Content item added successfully to workspace: ${workspace.name}")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error adding content item to workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Удаление элемента содержимого из рабочего пространства
    fun removeContentItem(workspace: Workspace, itemId: String) {
        Log.d("StartAppTag", "Removing content item from workspace: ${workspace.name}")
        try {
            repository.removeContentItem(workspace, itemId)
            if (_currentWorkspace.value?.id == workspace.id) {
                _currentWorkspace.value = repository.getWorkspaceById(workspace.id)
            }
            Log.d("StartAppTag", "Content item removed successfully from workspace: ${workspace.name}")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error removing content item from workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Обновление элемента содержимого в рабочем пространстве
    fun updateContentItem(workspace: Workspace, item: ContentItem) {
        Log.d("StartAppTag", "Updating content item in workspace: ${workspace.name}")
        try {
            repository.updateContentItem(workspace, item)
            if (_currentWorkspace.value?.id == workspace.id) {
                _currentWorkspace.value = repository.getWorkspaceById(workspace.id)
            }
            Log.d("StartAppTag", "Content item updated successfully in workspace: ${workspace.name}")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error updating content item in workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Получение списка избранных рабочих пространств
    fun getFavorites(): List<Workspace> {
        Log.d("StartAppTag", "Getting favorite workspaces")
        return workspaces.value?.filter { it.isFavorite } ?: emptyList()
    }
    
    // Переключение статуса "избранное" для рабочего пространства
    fun toggleFavorite(workspace: Workspace): Boolean {
        Log.d("StartAppTag", "Toggling favorite status for workspace: ${workspace.name}")
        try {
            workspace.isFavorite = !workspace.isFavorite
            repository.updateWorkspace(workspace)
            Log.d("StartAppTag", "Favorite status toggled successfully for workspace: ${workspace.name}")
            return workspace.isFavorite
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error toggling favorite status for workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Получение недавно использовавшихся рабочих пространств
    fun getRecentlyAccessed(limit: Int = 5): List<Workspace> {
        Log.d("StartAppTag", "Getting recently accessed workspaces")
        return workspaces.value?.sortedByDescending { it.lastAccessed }?.take(limit) ?: emptyList()
    }
}

class UriTypeAdapter : JsonSerializer<Uri>, JsonDeserializer<Uri> {
    override fun serialize(src: Uri?, typeOfSrc: Type?, context: JsonSerializationContext?): JsonElement {
        return JsonPrimitive(src?.toString())
    }

    override fun deserialize(json: JsonElement?, typeOfT: Type?, context: JsonDeserializationContext?): Uri {
        val uriString = json?.asString
        return if (uriString != null) {
            try {
                Uri.parse(uriString)
            } catch (e: Exception) {
                Log.e("MindNote", "Error parsing URI: $uriString", e)
                Uri.EMPTY
            }
        } else {
            Uri.EMPTY
        }
    }
}

class ContentItemTypeAdapter : JsonSerializer<ContentItem>, JsonDeserializer<ContentItem> {
    override fun serialize(src: ContentItem?, typeOfSrc: Type?, context: JsonSerializationContext?): JsonElement {
        val jsonObject = JsonObject()
        when (src) {
            is ContentItem.TextItem -> {
                jsonObject.addProperty("type", "TextItem")
                jsonObject.addProperty("text", src.text)
                jsonObject.addProperty("id", src.id)
            }
            is ContentItem.CheckboxItem -> {
                jsonObject.addProperty("type", "CheckboxItem")
                jsonObject.addProperty("text", src.text)
                jsonObject.addProperty("isChecked", src.isChecked)
                jsonObject.addProperty("id", src.id)
            }
            is ContentItem.NumberedListItem -> {
                jsonObject.addProperty("type", "NumberedListItem")
                jsonObject.addProperty("text", src.text)
                jsonObject.addProperty("number", src.number)
                jsonObject.addProperty("id", src.id)
            }
            is ContentItem.BulletListItem -> {
                jsonObject.addProperty("type", "BulletListItem")
                jsonObject.addProperty("text", src.text)
                jsonObject.addProperty("id", src.id)
            }
            is ContentItem.ImageItem -> {
                jsonObject.addProperty("type", "ImageItem")
                jsonObject.addProperty("imageUri", src.imageUri.toString())
                jsonObject.addProperty("id", src.id)
            }
            is ContentItem.FileItem -> {
                jsonObject.addProperty("type", "FileItem")
                jsonObject.addProperty("fileName", src.fileName)
                jsonObject.addProperty("fileUri", src.fileUri.toString())
                jsonObject.addProperty("fileSize", src.fileSize)
                jsonObject.addProperty("id", src.id)
            }
            is ContentItem.SubWorkspaceLink -> {
                jsonObject.addProperty("type", "SubWorkspaceLink")
                jsonObject.addProperty("workspaceId", src.workspaceId)
                jsonObject.addProperty("displayName", src.displayName)
                jsonObject.addProperty("id", src.id)
            }
            null -> return JsonObject()
        }
        return jsonObject
    }

    override fun deserialize(json: JsonElement?, typeOfT: Type?, context: JsonDeserializationContext?): ContentItem {
        val jsonObject = json?.asJsonObject ?: return ContentItem.TextItem("")
        val type = jsonObject.get("type")?.asString ?: return ContentItem.TextItem("")

        return when (type) {
            "TextItem" -> ContentItem.TextItem(
                text = jsonObject.get("text")?.asString ?: "",
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "CheckboxItem" -> ContentItem.CheckboxItem(
                text = jsonObject.get("text")?.asString ?: "",
                isChecked = jsonObject.get("isChecked")?.asBoolean ?: false,
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "NumberedListItem" -> ContentItem.NumberedListItem(
                text = jsonObject.get("text")?.asString ?: "",
                number = jsonObject.get("number")?.asInt ?: 0,
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "BulletListItem" -> ContentItem.BulletListItem(
                text = jsonObject.get("text")?.asString ?: "",
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "ImageItem" -> ContentItem.ImageItem(
                imageUri = Uri.parse(jsonObject.get("imageUri")?.asString),
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "FileItem" -> ContentItem.FileItem(
                fileName = jsonObject.get("fileName")?.asString ?: "",
                fileUri = Uri.parse(jsonObject.get("fileUri")?.asString),
                fileSize = jsonObject.get("fileSize")?.asLong ?: 0L,
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "SubWorkspaceLink" -> ContentItem.SubWorkspaceLink(
                workspaceId = jsonObject.get("workspaceId")?.asString ?: "",
                displayName = jsonObject.get("displayName")?.asString ?: "",
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            else -> ContentItem.TextItem("")
        }
    }
}

