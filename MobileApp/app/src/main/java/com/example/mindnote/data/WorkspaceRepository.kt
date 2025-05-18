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
        Log.d("Repository", "LOADWWORK")
        loadWorkspaces()
    }
    
    // Метод загрузки всех рабочих пространств
    fun loadWorkspaces() {
        Log.d("Repository", "LOADINWORK")
        try {
            val file = File(context.filesDir, "workspaces.json")
            if (file.exists()) {
                val json = file.readText()
                Log.d("Repository", "Loading workspaces: $json")
                val type = object : TypeToken<List<Workspace>>() {}.type
                Log.d("Repository", "Getting type")
                val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type) ?: emptyList()
                Log.d("Repository", "Getting gson")
                _workspaces.postValue(loadedWorkspaces)
                Log.d("Repository", "Loaded ${loadedWorkspaces.size} workspaces")
            } else {
                _workspaces.postValue(emptyList())
                Log.d("Repository", "No workspaces file found")
            }
        } catch (e: Exception) {
            Log.e("Repository", "Error loading workspaces", e)
            
            // Пробуем восстановить из резервной копии
            try {
                val backupFile = File(context.filesDir, "workspaces_backup.json")
                if (backupFile.exists()) {
                    val json = backupFile.readText()
                    val type = object : TypeToken<List<Workspace>>() {}.type
                    val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type) ?: emptyList()
                    _workspaces.postValue(loadedWorkspaces)
                    Log.d("Repository", "Restored from backup: ${loadedWorkspaces.size} workspaces")
                } else {
                    _workspaces.postValue(emptyList())
                }
            } catch (e2: Exception) {
                Log.e("Repository", "Error restoring from backup", e2)
                _workspaces.postValue(emptyList())
            }
        }
    }
    
    // Метод сохранения всех рабочих пространств
    fun saveWorkspaces() {
        try {
            val currentWorkspaces = _workspaces.value ?: emptyList()
            Log.d("Repository", "Saving ${currentWorkspaces.size} workspaces")
            
            // Создаем JSON
            val json = gson.toJson(currentWorkspaces)
            
            // Сохраняем в основной файл
            val file = File(context.filesDir, "workspaces.json")
            file.writeText(json)
            
            // Сохраняем резервную копию
            val backupFile = File(context.filesDir, "workspaces_backup.json")
            backupFile.writeText(json)

            Log.d("repos1", currentWorkspaces.get(0).items.size.toString())
            Log.d("repos1", currentWorkspaces.get(1).items.size.toString())
            Log.d("Repository", "Workspaces saved successfully with backup")
        } catch (e: Exception) {
            Log.e("Repository", "Error saving workspaces", e)
        }
    }
    
    // Создание нового рабочего пространства
    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        val workspace = Workspace(name, iconUri?.toString() ?: "")
        val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
        currentList.add(workspace)
        _workspaces.postValue(currentList)
        saveWorkspaces()
        return workspace
    }
    
    // Обновление существующего рабочего пространства
    fun updateWorkspace(workspace: Workspace) {
        val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
        val index = currentList.indexOfFirst { it.id == workspace.id }
        if (index != -1) {
            currentList[index] = workspace
            _workspaces.postValue(currentList)
            saveWorkspaces()
        }
    }
    
    // Получение рабочего пространства по имени
    fun getWorkspaceByName(name: String): Workspace? {
        return _workspaces.value?.find { it.name == name }
    }
    
    // Получение рабочего пространства по ID
    fun getWorkspaceById(id: String): Workspace? {
        return _workspaces.value?.find { it.id == id }
    }
    
    // Добавление элемента содержимого в рабочее пространство
    fun addContentItem(workspace: Workspace, item: ContentItem) {
        // Получаем актуальную версию из списка
        val current = getWorkspaceById(workspace.id) ?: workspace
        
        // Добавляем элемент
        current.addItem(item)
        
        // Обновляем рабочее пространство
        updateWorkspace(current)
        
        Log.d("Repository", "Added item to workspace '${current.name}', now has ${current.items.size} items")
    }
    
    // Удаление элемента содержимого из рабочего пространства
    fun removeContentItem(workspace: Workspace, itemId: String) {
        // Получаем актуальную версию из списка
        val current = getWorkspaceById(workspace.id) ?: workspace
        
        // Находим элемент для удаления
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
        
        // Удаляем элемент если найден
        if (itemToRemove != null) {
            current.removeItem(itemId)
            updateWorkspace(current)
            Log.d("Repository", "Removed item from workspace '${current.name}', now has ${current.items.size} items")
        }
    }
    
    // Обновление элемента содержимого в рабочем пространстве
    fun updateContentItem(workspace: Workspace, updatedItem: ContentItem) {
        // Получаем актуальную версию из списка
        val current = getWorkspaceById(workspace.id) ?: workspace
        
        // Находим индекс элемента для обновления
        val itemIndex = current.items.indexOfFirst { item ->
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
                is ContentItem.SubWorkspaceLink -> 
                    item.id == (updatedItem as? ContentItem.SubWorkspaceLink)?.id
            }
        }
        
        // Обновляем элемент если найден
        if (itemIndex != -1) {
            current.updateItem(updatedItem)
            updateWorkspace(current)
            Log.d("Repository", "Updated item in workspace '${current.name}'")
        }
    }
    
    companion object {
        @Volatile private var INSTANCE: WorkspaceRepository? = null
        
        fun getInstance(context: Context): WorkspaceRepository {
            return INSTANCE ?: synchronized(this) {
                INSTANCE ?: WorkspaceRepository(context.applicationContext).also { INSTANCE = it }
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