package com.example.mindnote

import android.content.Context
import android.net.Uri
import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.example.mindnote.data.ContentItem
import com.example.mindnote.data.Workspace
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
    private val _workspaces = MutableLiveData<List<Workspace>>()
    val workspaces: LiveData<List<Workspace>> = _workspaces
    
    private val _currentWorkspace = MutableLiveData<Workspace>()
    val currentWorkspace: LiveData<Workspace> = _currentWorkspace

    private val gson: Gson = GsonBuilder()
        .registerTypeAdapter(Uri::class.java, UriTypeAdapter())
        .registerTypeAdapter(ContentItem::class.java, ContentItemTypeAdapter())
        .setPrettyPrinting()
        .create()

    init {
        _workspaces.value = emptyList()
    }
    
    fun initWorkspaces(initialWorkspaces: List<Workspace>) {
        _workspaces.value = initialWorkspaces
    }

    fun loadWorkspaces(context: Context) {
        try {
            val file = File(context.filesDir, "workspaces.json")
            if (file.exists()) {
                val json = file.readText()
                Log.d("MindNote", "Loading workspaces: $json")
                val type = object : TypeToken<List<Workspace>>() {}.type
                val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type) ?: emptyList()
                _workspaces.value = loadedWorkspaces
                Log.d("MindNote", "Loaded ${loadedWorkspaces.size} workspaces")
                loadedWorkspaces.forEach { workspace ->
                    Log.d("MindNote", "Workspace '${workspace.name}' has ${workspace.items.size} items")
                }
            } else {
                _workspaces.value = emptyList()
                Log.d("MindNote", "No workspaces file found")
            }
        } catch (e: Exception) {
            Log.e("MindNote", "Error loading workspaces", e)
            e.printStackTrace()
            _workspaces.value = emptyList()
        }
    }

    fun saveWorkspaces(context: Context) {
        try {
            val file = File(context.filesDir, "workspaces.json")
            val currentWorkspaces = _workspaces.value ?: emptyList()
            Log.d("MindNote", "Saving ${currentWorkspaces.size} workspaces")
            currentWorkspaces.forEach { workspace ->
                Log.d("MindNote", "Workspace '${workspace.name}' has ${workspace.items.size} items")
            }
            val json = gson.toJson(currentWorkspaces)
            Log.d("MindNote", "Saving workspaces JSON: $json")
            file.writeText(json)
            Log.d("MindNote", "Workspaces saved successfully")
        } catch (e: Exception) {
            Log.e("MindNote", "Error saving workspaces", e)
            e.printStackTrace()
        }
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
        
        // Убедимся, что у нас самая актуальная версия workspace из списка
        val currentWorkspaces = _workspaces.value ?: emptyList()
        val updatedWorkspace = currentWorkspaces.find { it.id == workspace.id } ?: workspace
        
        // Обновим lastAccessed
        updatedWorkspace.lastAccessed = System.currentTimeMillis()
        
        // Обновим в списке
        updateWorkspace(updatedWorkspace)
        
        // Установим как текущее
        _currentWorkspace.value = updatedWorkspace
        
        Log.d("MindNote", "Set current workspace: ${updatedWorkspace.name} with ${updatedWorkspace.items.size} items")
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
        Log.d("MindNote", "Adding ${item.javaClass.simpleName} to workspace '${workspace.name}'")
        
        // Получим актуальную версию workspace из списка
        val currentWorkspaces = _workspaces.value ?: emptyList()
        val workspaceToUpdate = currentWorkspaces.find { it.id == workspace.id } ?: workspace
        
        // Обновим список элементов
        workspaceToUpdate.items.add(item)
        
        // Обновим в список рабочих пространств
        updateWorkspace(workspaceToUpdate)
        
        // Если это текущее рабочее пространство, обновим и его
        if (_currentWorkspace.value?.id == workspaceToUpdate.id) {
            _currentWorkspace.value = workspaceToUpdate
        }
        
        Log.d("MindNote", "Workspace '${workspaceToUpdate.name}' now has ${workspaceToUpdate.items.size} items")
    }

    fun removeContentItem(workspace: Workspace, itemId: String) {
        Log.d("MindNote", "Removing item $itemId from workspace '${workspace.name}'")
        
        // Получим актуальную версию workspace из списка
        val currentWorkspaces = _workspaces.value ?: emptyList()
        val workspaceToUpdate = currentWorkspaces.find { it.id == workspace.id } ?: workspace
        
        // Найдем индекс элемента для удаления
        val itemToRemove = workspaceToUpdate.items.find { item ->
            when (item) {
                is ContentItem.TextItem -> item.id == itemId
                is ContentItem.CheckboxItem -> item.id == itemId
                is ContentItem.NumberedListItem -> item.id == itemId
                is ContentItem.BulletListItem -> item.id == itemId
                is ContentItem.ImageItem -> item.id == itemId
                is ContentItem.FileItem -> item.id == itemId
            }
        }
        
        // Удалим элемент
        if (itemToRemove != null) {
            workspaceToUpdate.items.remove(itemToRemove)
        }
        
        // Обновим в список рабочих пространств
        updateWorkspace(workspaceToUpdate)
        
        // Если это текущее рабочее пространство, обновим и его
        if (_currentWorkspace.value?.id == workspaceToUpdate.id) {
            _currentWorkspace.value = workspaceToUpdate
        }
        
        Log.d("MindNote", "Workspace '${workspaceToUpdate.name}' now has ${workspaceToUpdate.items.size} items")
    }

    fun updateContentItem(workspace: Workspace, updatedItem: ContentItem) {
        Log.d("MindNote", "Updating item in workspace '${workspace.name}'")
        
        // Получим актуальную версию workspace из списка
        val currentWorkspaces = _workspaces.value ?: emptyList()
        val workspaceToUpdate = currentWorkspaces.find { it.id == workspace.id } ?: workspace
        
        // Найдем индекс элемента для обновления
        val itemIndex = workspaceToUpdate.items.indexOfFirst { item ->
            when (item) {
                is ContentItem.TextItem -> 
                    item.id == (updatedItem as? ContentItem.TextItem)?.id
                is ContentItem.CheckboxItem -> 
                    item.id == (updatedItem as? ContentItem.CheckboxItem)?.id
                is ContentItem.NumberedListItem -> 
                    item.id == (updatedItem as? ContentItem.NumberedListItem)?.id
                is ContentItem.BulletListItem -> 
                    item.id == (updatedItem as? ContentItem.BulletListItem)?.id
                is ContentItem.ImageItem -> 
                    item.id == (updatedItem as? ContentItem.ImageItem)?.id
                is ContentItem.FileItem -> 
                    item.id == (updatedItem as? ContentItem.FileItem)?.id
            }
        }
        
        // Обновим элемент
        if (itemIndex != -1) {
            workspaceToUpdate.items[itemIndex] = updatedItem
        }
        
        // Обновим в список рабочих пространств
        updateWorkspace(workspaceToUpdate)
        
        // Если это текущее рабочее пространство, обновим и его
        if (_currentWorkspace.value?.id == workspaceToUpdate.id) {
            _currentWorkspace.value = workspaceToUpdate
        }
        
        Log.d("MindNote", "Updated item in workspace '${workspaceToUpdate.name}'")
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

