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
 */
class WorkspaceRepository private constructor(private val context: Context) {
    private val _workspaces = MutableLiveData<List<Workspace>>(emptyList())
    val workspaces: LiveData<List<Workspace>> = _workspaces

    init {
        loadWorkspaces()  // Загружаем сохраненные рабочие пространства при инициализации
    }

    companion object {
        @Volatile
        private var INSTANCE: WorkspaceRepository? = null

        fun getInstance(context: Context): WorkspaceRepository {
            return INSTANCE ?: synchronized(this) {
                try {
                    if (context == null) {
                        throw IllegalArgumentException("Context cannot be null")
                    }
                    INSTANCE ?: WorkspaceRepository(context.applicationContext).also { INSTANCE = it }
                } catch (e: Exception) {
                    Log.e("MindNote", "Error creating WorkspaceRepository instance", e)
                    throw e
                }
            }
        }
    }

    // Создание нового рабочего пространства
    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        val workspace = Workspace(name, iconUri)
        val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
        currentList.add(workspace)
        _workspaces.value = currentList // Используем value вместо postValue для синхронного обновления
        Log.d("MindNote", """
            Created new workspace:
            - Name: ${workspace.name}
            - ID: ${workspace.id}
            - Created at: ${workspace.created_at}
            - Icon URI: ${workspace.iconUri}
        """.trimIndent())
        saveWorkspaces() // Сохраняем сразу после создания
        return workspace
    }
    
    // Обновление существующего рабочего пространства
    fun updateWorkspace(workspace: Workspace) {
        val currentWorkspaces = _workspaces.value?.toMutableList() ?: mutableListOf()
        val index = currentWorkspaces.indexOfFirst { it.id == workspace.id }
        if (index != -1) {
            currentWorkspaces[index] = workspace
            _workspaces.value = currentWorkspaces // Используем value для синхронного обновления
            Log.d("MindNote", """
                Updated workspace:
                - Name: ${workspace.name}
                - ID: ${workspace.id}
                - Created at: ${workspace.created_at}
                - Last accessed: ${workspace.lastAccessed}
                - Items count: ${workspace.items.size}
            """.trimIndent())
            saveWorkspaces() // Сохраняем сразу после обновления
        }
    }
    
    // Получение рабочего пространства по имени
    fun getWorkspaceByName(name: String): Workspace? {
        return _workspaces.value?.find { it.name == name }
    }
    
    // Получение рабочего пространства по ID
    fun getWorkspaceById(id: String): Workspace? {
        val workspace = _workspaces.value?.find { it.id == id }
        Log.d("MindNote", "WorkspaceRepository: Looking for workspace with ID '$id', found: ${workspace?.name}, total workspaces: ${_workspaces.value?.size}")
        return workspace
    }

    // Добавление элемента содержимого в рабочее пространство
    fun addContentItem(workspace: Workspace, item: ContentItem) {
        val currentWorkspace = _workspaces.value?.find { it.id == workspace.id }
        currentWorkspace?.let {
            it.addItem(item)
            updateWorkspace(it)
            
            // Если это вложенное пространство, обновляем родительское
            if (item is ContentItem.NestedPageItem) {
                val parentWorkspace = findParentWorkspace(it)
                parentWorkspace?.let { parent ->
                    Log.d("MindNote", "Updating parent workspace ${parent.name} after adding nested page ${item.pageName}")
                    updateWorkspace(parent)
                }
            }
            
            Log.d("MindNote", "Added item ${item.id} to workspace ${workspace.name}")
        }
    }
    
    // Удаление элемента содержимого из рабочего пространства
    fun removeContentItem(workspace: Workspace, itemId: String) {
        try {
            val currentWorkspace = _workspaces.value?.find { it.id == workspace.id }
            currentWorkspace?.let {
                // Находим элемент перед удалением
                val itemToRemove = it.items.find { item -> item.id == itemId }
                
                // Если это ссылка на подпространство, удаляем и само подпространство
                if (itemToRemove is ContentItem.NestedPageItem) {
                    val nestedWorkspace = getWorkspaceById(itemToRemove.pageId)
                    nestedWorkspace?.let { nested ->
                        Log.d("MindNote", "Removing nested workspace ${nested.name} along with its reference")
                        deleteWorkspace(nested)
                    }
                }
                
                it.removeItem(itemId)
                updateWorkspace(it)
                
                // Если это вложенное пространство, обновляем родительское
                val parentWorkspace = findParentWorkspace(it)
                parentWorkspace?.let { parent ->
                    Log.d("MindNote", "Updating parent workspace ${parent.name} after removing item $itemId")
                    updateWorkspace(parent)
                }
                
                Log.d("MindNote", "Removed item $itemId from workspace ${workspace.name}")
            }
        } catch (e: Exception) {
            Log.e("MindNote", "Error removing content item in repository", e)
            throw e
        }
    }
    
    // Обновление элемента содержимого в рабочем пространстве
    fun updateContentItem(workspace: Workspace, item: ContentItem) {
        val currentWorkspace = _workspaces.value?.find { it.id == workspace.id }
        currentWorkspace?.let {
            when (item) {
                is ContentItem.TextItem -> it.updateItem(item)
                is ContentItem.CheckboxItem -> it.updateItem(item)
                is ContentItem.ImageItem -> it.updateItem(item)
                is ContentItem.FileItem -> it.updateItem(item)
                is ContentItem.NestedPageItem -> it.updateItem(item)
            }
            updateWorkspace(it)
            
            // Если это вложенное пространство, обновляем родительское
            val parentWorkspace = findParentWorkspace(it)
            parentWorkspace?.let { parent ->
                Log.d("MindNote", "Updating parent workspace ${parent.name} after updating item ${item.id}")
                updateWorkspace(parent)
            }
            
            Log.d("MindNote", "Updated item ${item.id} in workspace ${workspace.name}")
        }
    }

    // Поиск родительского рабочего пространства
    private fun findParentWorkspace(workspace: Workspace): Workspace? {
        return _workspaces.value?.find { parent ->
            parent.items.any { item ->
                item is ContentItem.NestedPageItem && item.pageId == workspace.id
            }
        }
    }

    private fun getUserFolder(): File {
        val userName = UserManager.getCurrentUserName()
        val userFolder = File(context.filesDir, userName)
        if (!userFolder.exists()) {
            userFolder.mkdirs()
            Log.d("MindNote", "Created user folder: ${userFolder.absolutePath}")
        } else {
            Log.d("MindNote", "Using existing user folder: ${userFolder.absolutePath}")
        }
        return userFolder
    }

    // Загрузка сохраненных рабочих пространств
    private fun loadWorkspaces() {
        val gson = GsonBuilder()
            .registerTypeAdapter(Uri::class.java, UriTypeAdapter())
            .registerTypeAdapter(ContentItem::class.java, ContentItemTypeAdapter())
            .create()

        try {
            val file = File(getUserFolder(), "workspaces.json")
            Log.d("MindNote", "Attempting to load workspaces from: ${file.absolutePath}")
            
            if (file.exists()) {
                val json = file.readText()
                Log.d("MindNote", "Loaded JSON content: $json")
                
                val type = object : TypeToken<List<Workspace>>() {}.type
                val loadedWorkspaces = gson.fromJson<List<Workspace>>(json, type)
                
                // Log details of each loaded workspace
                loadedWorkspaces.forEach { workspace ->
                    Log.d("MindNote", """
                        Loaded workspace:
                        - Name: ${workspace.name}
                        - ID: ${workspace.id}
                        - Created at: ${workspace.created_at}
                        - Last accessed: ${workspace.lastAccessed}
                        - Items count: ${workspace.items.size}
                        - Items: ${workspace.items.joinToString { 
                            when (it) {
                                is ContentItem.TextItem -> "TextItem(id=${it.id})"
                                is ContentItem.CheckboxItem -> "CheckboxItem(id=${it.id})"
                                is ContentItem.ImageItem -> "ImageItem(id=${it.id})"
                                is ContentItem.FileItem -> "FileItem(id=${it.id}, name=${it.fileName})"
                                is ContentItem.NestedPageItem -> "NestedPageItem(id=${it.id}, pageId=${it.pageId})"
                            }
                        }}
                    """.trimIndent())
                }
                
                _workspaces.value = loadedWorkspaces
                Log.d("MindNote", "Successfully loaded ${loadedWorkspaces.size} workspaces")
            } else {
                Log.d("MindNote", "No workspaces file found at: ${file.absolutePath}")
            }
        } catch (e: Exception) {
            Log.e("MindNote", "Error loading workspaces from ${getUserFolder().absolutePath}", e)
        }
    }

    // Сохранение всех рабочих пространств
    fun saveWorkspaces() {
        val gson = GsonBuilder()
            .registerTypeAdapter(Uri::class.java, UriTypeAdapter())
            .registerTypeAdapter(ContentItem::class.java, ContentItemTypeAdapter())
            .create()

        try {
            val workspaces = _workspaces.value
            if (workspaces == null || workspaces.isEmpty()) {
                Log.w("MindNote", "No workspaces to save or empty list")
                return
            }

            // Log details of each workspace being saved
            workspaces.forEach { workspace ->
                Log.d("MindNote", """
                    Saving workspace:
                    - Name: ${workspace.name}
                    - ID: ${workspace.id}
                    - Created at: ${workspace.created_at}
                    - Last accessed: ${workspace.lastAccessed}
                    - Items count: ${workspace.items.size}
                    - Items: ${workspace.items.joinToString { 
                        when (it) {
                            is ContentItem.TextItem -> "TextItem(id=${it.id})"
                            is ContentItem.CheckboxItem -> "CheckboxItem(id=${it.id})"
                            is ContentItem.ImageItem -> "ImageItem(id=${it.id})"
                            is ContentItem.FileItem -> "FileItem(id=${it.id}, name=${it.fileName})"
                            is ContentItem.NestedPageItem -> "NestedPageItem(id=${it.id}, pageId=${it.pageId})"
                        }
                    }}
                """.trimIndent())
            }

            val json = gson.toJson(workspaces)
            val file = File(getUserFolder(), "workspaces.json")
            Log.d("MindNote", "Saving workspaces to: ${file.absolutePath}")
            Log.d("MindNote", "JSON content to save: $json")
            
            file.writeText(json)
            Log.d("MindNote", "Successfully saved ${workspaces.size} workspaces")
        } catch (e: Exception) {
            Log.e("MindNote", "Error saving workspaces to ${getUserFolder().absolutePath}", e)
        }
    }

    // Удаление рабочего пространства
    fun deleteWorkspace(workspace: Workspace) {
        try {
            val currentWorkspaces = _workspaces.value?.toMutableList() ?: mutableListOf()
            currentWorkspaces.removeIf { it.id == workspace.id }
            _workspaces.value = currentWorkspaces // Используем value для синхронного обновления
            Log.d("MindNote", """
                Deleted workspace:
                - Name: ${workspace.name}
                - ID: ${workspace.id}
                - Created at: ${workspace.created_at}
                - Last accessed: ${workspace.lastAccessed}
            """.trimIndent())
            saveWorkspaces() // Сохраняем сразу после удаления
        } catch (e: Exception) {
            Log.e("MindNote", "Error deleting workspace ${workspace.name}", e)
            throw e
        }
    }

    // Перезагрузка рабочих пространств при смене пользователя
    fun reloadWorkspaces() {
        loadWorkspaces()
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
                jsonObject.addProperty("isFormatted", src.isFormatted)
                if (src.isFormatted) {
                    jsonObject.addProperty("htmlText", src.htmlText)
                }
            }
            is ContentItem.CheckboxItem -> {
                jsonObject.addProperty("type", "CheckboxItem")
                jsonObject.addProperty("text", src.text)
                jsonObject.addProperty("isChecked", src.isChecked)
                jsonObject.addProperty("id", src.id)
                jsonObject.addProperty("isFormatted", src.isFormatted)
                if (src.isFormatted) {
                    jsonObject.addProperty("htmlText", src.htmlText)
                }
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
            is ContentItem.NestedPageItem -> {
                jsonObject.addProperty("type", "NestedPageItem")
                jsonObject.addProperty("pageName", src.pageName)
                jsonObject.addProperty("pageId", src.pageId)
                jsonObject.addProperty("id", src.id)
                src.iconUri?.let { jsonObject.addProperty("iconUri", it.toString()) }
            }
            null -> return JsonObject()
        }
        return jsonObject
    }

    override fun deserialize(json: JsonElement?, typeOfT: Type?, context: JsonDeserializationContext?): ContentItem {
        val jsonObject = json?.asJsonObject ?: return ContentItem.TextItem("")
        val type = jsonObject.get("type")?.asString ?: return ContentItem.TextItem("")

        return when (type) {
            "TextItem" -> {
                val isFormatted = jsonObject.get("isFormatted")?.asBoolean ?: false
                val text = jsonObject.get("text")?.asString ?: ""
                val htmlText = if (isFormatted) {
                    jsonObject.get("htmlText")?.asString ?: text
                } else {
                    text
                }
                ContentItem.TextItem(
                    text = text,
                    htmlText = htmlText,
                    isFormatted = isFormatted,
                    id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
                )
            }
            "CheckboxItem" -> {
                val isFormatted = jsonObject.get("isFormatted")?.asBoolean ?: false
                val text = jsonObject.get("text")?.asString ?: ""
                val htmlText = if (isFormatted) {
                    jsonObject.get("htmlText")?.asString ?: text
                } else {
                    text
                }
                ContentItem.CheckboxItem(
                    text = text,
                    htmlText = htmlText,
                    isFormatted = isFormatted,
                    isChecked = jsonObject.get("isChecked")?.asBoolean ?: false,
                    id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
                )
            }
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
            "NestedPageItem" -> ContentItem.NestedPageItem(
                pageName = jsonObject.get("pageName")?.asString ?: "",
                pageId = jsonObject.get("pageId")?.asString ?: java.util.UUID.randomUUID().toString(),
                iconUri = jsonObject.get("iconUri")?.asString?.let { Uri.parse(it) },
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            else -> ContentItem.TextItem("")
        }
    }
} 