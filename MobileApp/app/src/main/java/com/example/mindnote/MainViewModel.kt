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
        repository = WorkspaceRepository.getInstance(context)
    }
    
    // LiveData со списком всех рабочих пространств
    val workspaces: LiveData<List<Workspace>> 
        get() = repository.workspaces
    
    // Создание нового рабочего пространства
    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        return repository.createWorkspace(name, iconUri)
    }
    
    // Обновление рабочего пространства
    fun updateWorkspace(workspace: Workspace) {
        repository.updateWorkspace(workspace)
    }
    
    // Получение рабочего пространства по имени
    fun getWorkspaceByName(name: String): Workspace? {
        return repository.getWorkspaceByName(name)
    }
    
    // Получение рабочего пространства по ID
    fun getWorkspaceById(id: String): Workspace? {
        return repository.getWorkspaceById(id)
    }
    
    // Установка текущего рабочего пространства
    fun setCurrentWorkspace(workspace: Workspace) {
        _currentWorkspace.value = workspace
    }
    
    // Добавление нового элемента содержимого в рабочее пространство
    fun addContentItem(workspace: Workspace, item: ContentItem) {
        repository.addContentItem(workspace, item)
        // Если это текущее рабочее пространство, обновляем его
        if (_currentWorkspace.value?.id == workspace.id) {
            _currentWorkspace.value = repository.getWorkspaceById(workspace.id)
        }
    }
    
    // Удаление элемента содержимого из рабочего пространства
    fun removeContentItem(workspace: Workspace, itemId: String) {
        repository.removeContentItem(workspace, itemId)
        // Если это текущее рабочее пространство, обновляем его
        if (_currentWorkspace.value?.id == workspace.id) {
            _currentWorkspace.value = repository.getWorkspaceById(workspace.id)
        }
    }
    
    // Обновление элемента содержимого в рабочем пространстве
    fun updateContentItem(workspace: Workspace, item: ContentItem) {
        repository.updateContentItem(workspace, item)
        // Если это текущее рабочее пространство, обновляем его
        if (_currentWorkspace.value?.id == workspace.id) {
            _currentWorkspace.value = repository.getWorkspaceById(workspace.id)
        }
    }
    
    // Получение списка избранных рабочих пространств
    fun getFavorites(): List<Workspace> {
        return workspaces.value?.filter { it.isFavorite } ?: emptyList()
    }
    
    // Переключение статуса "избранное" для рабочего пространства
    fun toggleFavorite(workspace: Workspace) {
        workspace.isFavorite = !workspace.isFavorite
        repository.updateWorkspace(workspace)
    }
    
    // Получение недавно использовавшихся рабочих пространств
    fun getRecentlyAccessed(limit: Int = 5): List<Workspace> {
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
            else -> ContentItem.TextItem("")
        }
    }
}

