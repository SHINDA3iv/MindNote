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

    companion object {
        @Volatile
        private var INSTANCE: WorkspaceRepository? = null

        fun getInstance(context: Context): WorkspaceRepository {
            return INSTANCE ?: synchronized(this) {
                INSTANCE ?: WorkspaceRepository(context).also { INSTANCE = it }
            }
        }
    }

    // Создание нового рабочего пространства
    fun createWorkspace(name: String, iconUri: Uri? = null): Workspace {
        val workspace = Workspace(name, iconUri)
        val currentList = _workspaces.value?.toMutableList() ?: mutableListOf()
        currentList.add(workspace)
        _workspaces.postValue(currentList)
        return workspace
    }
    
    // Обновление существующего рабочего пространства
    fun updateWorkspace(workspace: Workspace) {
        val currentWorkspaces = _workspaces.value?.toMutableList() ?: mutableListOf()
        val index = currentWorkspaces.indexOfFirst { it.id == workspace.id }
        if (index != -1) {
            currentWorkspaces[index] = workspace
            _workspaces.value = currentWorkspaces
            Log.d("MindNote", "WorkspaceRepository: Updated workspace ${workspace.name}")
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
            Log.d("MindNote", "WorkspaceRepository: Added item ${item.id} to workspace ${workspace.name}")
        }
    }
    
    // Удаление элемента содержимого из рабочего пространства
    fun removeContentItem(workspace: Workspace, itemId: String) {
        val currentWorkspace = _workspaces.value?.find { it.id == workspace.id }
        currentWorkspace?.let {
            it.removeItem(itemId)
            updateWorkspace(it)
            Log.d("MindNote", "WorkspaceRepository: Removed item $itemId from workspace ${workspace.name}")
        }
    }
    
    // Обновление элемента содержимого в рабочем пространстве
    fun updateContentItem(workspace: Workspace, item: ContentItem) {
        when (item) {
            is ContentItem.TextItem -> workspace.updateItem(item)
            is ContentItem.CheckboxItem -> workspace.updateItem(item)
            is ContentItem.ImageItem -> workspace.updateItem(item)
            is ContentItem.FileItem -> workspace.updateItem(item)
            is ContentItem.NestedPageItem -> workspace.updateItem(item)
        }
        updateWorkspace(workspace)
    }

    // Сохранение всех рабочих пространств
    fun saveWorkspaces() {
        val gson = GsonBuilder()
            .registerTypeAdapter(Uri::class.java, UriTypeAdapter())
            .registerTypeAdapter(ContentItem::class.java, ContentItemTypeAdapter())
            .create()

        try {
            val json = gson.toJson(_workspaces.value)
            val file = File(context.filesDir, "workspaces.json")
            file.writeText(json)
            Log.d("MindNote", "Workspaces saved successfully")
        } catch (e: Exception) {
            Log.e("MindNote", "Error saving workspaces", e)
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
                id = jsonObject.get("id")?.asString ?: java.util.UUID.randomUUID().toString()
            )
            else -> ContentItem.TextItem("")
        }
    }
} 