package com.example.mindnote.data

import android.content.Context
import android.net.Uri
import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
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

/**
 * Централизованное хранилище всех данных рабочих пространств
 * Обеспечивает надежное сохранение и загрузку данных
 */
class WorkspaceRepository private constructor(private val context: Context) {
    private val _workspaces = MutableLiveData<List<Workspace>>()
    val workspaces: LiveData<List<Workspace>> = _workspaces
    
    private val gson: Gson = GsonBuilder()
        .registerTypeAdapter(Uri::class.java, UriTypeAdapter())
        .registerTypeAdapter(ContentItem::class.java, ContentItemTypeAdapter())
        .setPrettyPrinting()
        .create()
    
    init {
        Log.d("StartAppTag", "Initializing WorkspaceRepository")
        try {
            loadWorkspaces()
            Log.d("StartAppTag", "WorkspaceRepository initialized successfully")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error initializing WorkspaceRepository", e)
            throw e
        }
    }
    
    // Метод загрузки всех рабочих пространств
    fun loadWorkspaces() {
        Log.d("StartAppTag", "Loading workspaces from storage")
        try {
            val file = File(context.filesDir, "workspaces.json")
            if (file.exists()) {
                val json = file.readText()
                Log.d("StartAppTag", "Found workspaces file, size: ${json.length} bytes")
                val type = object : TypeToken<List<Workspace>>() {}.type
                val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type) ?: emptyList()
                _workspaces.postValue(loadedWorkspaces)
                Log.d("StartAppTag", "Successfully loaded ${loadedWorkspaces.size} workspaces")
            } else {
                _workspaces.postValue(emptyList())
                Log.d("StartAppTag", "No workspaces file found, initializing empty list")
            }
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error loading workspaces from main file", e)
            
            try {
                Log.d("StartAppTag", "Attempting to restore from backup")
                val backupFile = File(context.filesDir, "workspaces_backup.json")
                if (backupFile.exists()) {
                    val json = backupFile.readText()
                    val type = object : TypeToken<List<Workspace>>() {}.type
                    val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type) ?: emptyList()
                    _workspaces.postValue(loadedWorkspaces)
                    Log.d("StartAppTag", "Successfully restored ${loadedWorkspaces.size} workspaces from backup")
                } else {
                    _workspaces.postValue(emptyList())
                    Log.d("StartAppTag", "No backup file found, initializing empty list")
                }
            } catch (e2: Exception) {
                Log.e("StartAppTag", "Error restoring from backup", e2)
                _workspaces.postValue(emptyList())
            }
        }
    }
    
    // Метод сохранения всех рабочих пространств
    fun saveWorkspaces() {
        Log.d("StartAppTag", "Saving workspaces to storage")
        try {
            val currentWorkspaces = _workspaces.value ?: emptyList()
            Log.d("StartAppTag", "Preparing to save ${currentWorkspaces.size} workspaces")
            
            val json = gson.toJson(currentWorkspaces)
            
            val file = File(context.filesDir, "workspaces.json")
            file.writeText(json)
            
            val backupFile = File(context.filesDir, "workspaces_backup.json")
            backupFile.writeText(json)

            Log.d("StartAppTag", "Successfully saved workspaces to main file and backup")
            if (currentWorkspaces.isNotEmpty()) {
                Log.d("StartAppTag", "First workspace has ${currentWorkspaces[0].items.size} items")
                if (currentWorkspaces.size > 1) {
                    Log.d("StartAppTag", "Second workspace has ${currentWorkspaces[1].items.size} items")
                }
            }
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error saving workspaces", e)
            throw e
        }
    }
    
    // Создание нового рабочего пространства
    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        Log.d("StartAppTag", "Creating new workspace: $name")
        try {
            val workspace = Workspace(name, iconUri?.toString() ?: "")
            val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
            currentList.add(workspace)
            _workspaces.postValue(currentList)
            saveWorkspaces()
            Log.d("StartAppTag", "Successfully created new workspace: $name")
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
            val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
            val index = currentList.indexOfFirst { it.id == workspace.id }
            if (index != -1) {
                currentList[index] = workspace
                _workspaces.postValue(currentList)
                saveWorkspaces()
                Log.d("StartAppTag", "Successfully updated workspace: ${workspace.name}")
            } else {
                Log.w("StartAppTag", "Workspace not found for update: ${workspace.name}")
            }
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error updating workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Получение рабочего пространства по имени
    fun getWorkspaceByName(name: String): Workspace? {
        Log.d("StartAppTag", "Getting workspace by name: $name")
        return try {
            val workspace = _workspaces.value?.find { it.name == name }
            if (workspace != null) {
                Log.d("StartAppTag", "Found workspace: $name")
            } else {
                Log.d("StartAppTag", "Workspace not found: $name")
            }
            workspace
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error getting workspace by name: $name", e)
            null
        }
    }
    
    // Получение рабочего пространства по ID
    fun getWorkspaceById(id: String): Workspace? {
        Log.d("StartAppTag", "Getting workspace by ID: $id")
        return try {
            val workspace = _workspaces.value?.find { it.id == id }
            if (workspace != null) {
                Log.d("StartAppTag", "Found workspace with ID: $id")
            } else {
                Log.d("StartAppTag", "Workspace not found with ID: $id")
            }
            workspace
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error getting workspace by ID: $id", e)
            null
        }
    }
    
    // Добавление элемента содержимого в рабочее пространство
    fun addContentItem(workspace: Workspace, item: ContentItem) {
        Log.d("StartAppTag", "Adding content item to workspace: ${workspace.name}")
        try {
            val current = getWorkspaceById(workspace.id) ?: workspace
            current.addItem(item)
            updateWorkspace(current)
            Log.d("StartAppTag", "Successfully added item to workspace '${current.name}', now has ${current.items.size} items")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error adding content item to workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Удаление элемента содержимого из рабочего пространства
    fun removeContentItem(workspace: Workspace, itemId: String) {
        Log.d("StartAppTag", "Removing content item from workspace: ${workspace.name}")
        try {
            val current = getWorkspaceById(workspace.id) ?: workspace
            val itemToRemove = current.items.find { item ->
                when (item) {
                    is ContentItem.TextItem -> item.id == itemId
                    is ContentItem.CheckboxItem -> item.id == itemId
                    is ContentItem.NumberedListItem -> item.id == itemId
                    is ContentItem.BulletListItem -> item.id == itemId
                    is ContentItem.ImageItem -> item.id == itemId
                    is ContentItem.FileItem -> item.id == itemId
                    is ContentItem.SubWorkspaceLink -> item.id == itemId
                }
            }
            
            if (itemToRemove != null) {
                current.removeItem(itemId)
                updateWorkspace(current)
                Log.d("StartAppTag", "Successfully removed item from workspace '${current.name}', now has ${current.items.size} items")
            } else {
                Log.w("StartAppTag", "Item not found for removal in workspace: ${workspace.name}")
            }
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error removing content item from workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    // Обновление элемента содержимого в рабочем пространстве
    fun updateContentItem(workspace: Workspace, updatedItem: ContentItem) {
        Log.d("StartAppTag", "Updating content item in workspace: ${workspace.name}")
        try {
            val current = getWorkspaceById(workspace.id) ?: workspace
            current.updateItem(updatedItem)
            updateWorkspace(current)
            Log.d("StartAppTag", "Successfully updated content item in workspace: ${workspace.name}")
        } catch (e: Exception) {
            Log.e("StartAppTag", "Error updating content item in workspace: ${workspace.name}", e)
            throw e
        }
    }
    
    companion object {
        @Volatile
        private var INSTANCE: WorkspaceRepository? = null
        
        fun getInstance(context: Context): WorkspaceRepository {
            Log.d("StartAppTag", "Getting WorkspaceRepository instance")
            return INSTANCE ?: synchronized(this) {
                Log.d("StartAppTag", "Creating new WorkspaceRepository instance")
                val instance = WorkspaceRepository(context)
                INSTANCE = instance
                instance
            }
        }
    }
}

// Адаптеры для сериализации

class UriTypeAdapter : JsonSerializer<Uri>, JsonDeserializer<Uri> {
    override fun serialize(src: Uri?, typeOfSrc: Type?, context: JsonSerializationContext?): JsonElement {
        return JsonPrimitive(src?.toString() ?: "")
    }

    override fun deserialize(json: JsonElement?, typeOfT: Type?, context: JsonDeserializationContext?): Uri {
        val uriString = json?.asString
        return if (uriString != null && uriString.isNotEmpty()) {
            try {
                Uri.parse(uriString)
            } catch (e: Exception) {
                Log.e("Repository", "Error parsing URI: $uriString", e)
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
                imageUri = jsonObject.get("imageUri")?.asString?.let { Uri.parse(it) } ?: Uri.EMPTY,
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            "FileItem" -> ContentItem.FileItem(
                fileName = jsonObject.get("fileName")?.asString ?: "",
                fileUri = jsonObject.get("fileUri")?.asString?.let { Uri.parse(it) } ?: Uri.EMPTY,
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