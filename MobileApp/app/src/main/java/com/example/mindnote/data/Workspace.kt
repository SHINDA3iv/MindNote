package com.example.mindnote.data

import android.net.Uri
import android.util.Log

data class Workspace(
    var name: String,
    var iconUri: Uri? = null,
    var isFavorite: Boolean = false,
    var lastAccessed: Long = System.currentTimeMillis(),
    private val _items: MutableList<ContentItem> = mutableListOf(),
    var id: String = java.util.UUID.randomUUID().toString()
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
        val initialSize = _items.size
        
        // Находим элемент для удаления
        val itemToRemove = _items.find { item ->
            when (item) {
                is ContentItem.TextItem -> item.id == itemId
                is ContentItem.CheckboxItem -> item.id == itemId
                is ContentItem.ImageItem -> item.id == itemId
                is ContentItem.FileItem -> item.id == itemId
            }
        }
        
        // Удаляем элемент если найден
        if (itemToRemove != null) {
            _items.remove(itemToRemove)
            Log.d("Workspace", "Item removed from workspace '$name', now has ${_items.size} items")
        } else {
            Log.d("Workspace", "Item with ID $itemId not found in workspace '$name'")
        }
    }
    
    // Обновление элемента содержимого
    fun updateItem(updatedItem: ContentItem) {
        val index = _items.indexOfFirst { item ->
            when (item) {
                is ContentItem.TextItem -> 
                    item.id == (updatedItem as? ContentItem.TextItem)?.id
                is ContentItem.CheckboxItem -> 
                    item.id == (updatedItem as? ContentItem.CheckboxItem)?.id
                is ContentItem.ImageItem -> 
                    item.id == (updatedItem as? ContentItem.ImageItem)?.id
                is ContentItem.FileItem -> 
                    item.id == (updatedItem as? ContentItem.FileItem)?.id
            }
        }
        
        if (index != -1) {
            _items[index] = updatedItem
            val itemId = when (updatedItem) {
                is ContentItem.TextItem -> updatedItem.id
                is ContentItem.CheckboxItem -> updatedItem.id
                is ContentItem.ImageItem -> updatedItem.id
                is ContentItem.FileItem -> updatedItem.id
            }
            Log.d("Workspace", "Item updated in workspace '$name': id=$itemId, type=${updatedItem.javaClass.simpleName}")
        } else {
            val itemId = when (updatedItem) {
                is ContentItem.TextItem -> updatedItem.id
                is ContentItem.CheckboxItem -> updatedItem.id
                is ContentItem.ImageItem -> updatedItem.id
                is ContentItem.FileItem -> updatedItem.id
            }
            Log.d("Workspace", "Item with ID $itemId not found in workspace '$name', adding as new item")
            _items.add(updatedItem)
        }
    }
} 