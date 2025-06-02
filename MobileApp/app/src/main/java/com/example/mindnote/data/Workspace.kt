package com.example.mindnote.data

import android.net.Uri
import android.util.Log

data class Workspace(
    var name: String,
    var iconUri: Uri? = null,
    var isFavorite: Boolean = false,
    var lastAccessed: Long = System.currentTimeMillis(),
    private val _items: MutableList<ContentItem> = mutableListOf(),
    var id: String = java.util.UUID.randomUUID().toString(),
    var created_at: Long = System.currentTimeMillis()
) {
    // Создаем копию списка элементов при каждом доступе
    val items: List<ContentItem>
        get() = _items.toList()
    
    // Добавление элемента содержимого
    fun addItem(item: ContentItem) {
        _items.add(item)
        Log.d("Workspace", "Item added to workspace '$name', now has ${_items.size} items")
    }
    
    // Удаление элемента содержимого
    fun removeItem(itemId: String) {
        val itemToRemove = _items.find { item ->
            when (item) {
                is ContentItem.TextItem -> item.id == itemId
                is ContentItem.CheckboxItem -> item.id == itemId
                is ContentItem.ImageItem -> item.id == itemId
                is ContentItem.FileItem -> item.id == itemId
                is ContentItem.NestedPageItem -> item.id == itemId
            }
        }
        
        if (itemToRemove != null) {
            _items.remove(itemToRemove)
            Log.d("Workspace", "Item removed from workspace '$name', now has ${_items.size} items")
        }
    }
    
    // Обновление элемента содержимого
    fun updateItem(item: ContentItem) {
        val index = _items.indexOfFirst { existingItem ->
            when (existingItem) {
                is ContentItem.TextItem -> existingItem.id == item.id
                is ContentItem.CheckboxItem -> existingItem.id == item.id
                is ContentItem.ImageItem -> existingItem.id == item.id
                is ContentItem.FileItem -> existingItem.id == item.id
                is ContentItem.NestedPageItem -> existingItem.id == item.id
            }
        }
        
        if (index != -1) {
            _items[index] = item
            val itemId = when (item) {
                is ContentItem.TextItem -> item.id
                is ContentItem.CheckboxItem -> item.id
                is ContentItem.ImageItem -> item.id
                is ContentItem.FileItem -> item.id
                is ContentItem.NestedPageItem -> item.id
            }
            Log.d("Workspace", "Item updated in workspace '$name': id=$itemId, type=${item.javaClass.simpleName}")
        } else {
            val itemId = when (item) {
                is ContentItem.TextItem -> item.id
                is ContentItem.CheckboxItem -> item.id
                is ContentItem.ImageItem -> item.id
                is ContentItem.FileItem -> item.id
                is ContentItem.NestedPageItem -> item.id
            }
            Log.d("Workspace", "Item with ID $itemId not found in workspace '$name', adding as new item")
            _items.add(item)
        }
    }

    fun getItemById(id: String): ContentItem? {
        return _items.find { item ->
            when (item) {
                is ContentItem.TextItem -> item.id == id
                is ContentItem.CheckboxItem -> item.id == id
                is ContentItem.ImageItem -> item.id == id
                is ContentItem.FileItem -> item.id == id
                is ContentItem.NestedPageItem -> item.id == id
            }
        }
    }
} 