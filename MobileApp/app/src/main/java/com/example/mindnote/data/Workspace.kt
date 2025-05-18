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
                is ContentItem.NumberedListItem -> item.id == itemId
                is ContentItem.BulletListItem -> item.id == itemId
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
        // Находим индекс элемента для обновления
        val itemIndex = _items.indexOfFirst { item ->
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
        
        // Обновляем элемент если найден
        if (itemIndex != -1) {
            _items[itemIndex] = updatedItem
            Log.d("Workspace", "Item updated in workspace '$name' at position $itemIndex")
        } else {
            Log.d("Workspace", "Item not found for update in workspace '$name'")
        }
    }
    
    // Создаем копию рабочего пространства с новым списком элементов
//    fun copy(
//        name: String = this.name,
//        iconUri: Uri? = this.iconUri,
//        isFavorite: Boolean = this.isFavorite,
//        lastAccessed: Long = this.lastAccessed,
//        items: List<ContentItem> = this.items,
//        id: String = this.id
//    ): Workspace {
//        return Workspace(
//            name = name,
//            iconUri = iconUri,
//            isFavorite = isFavorite,
//            lastAccessed = lastAccessed,
//            _items = items.toMutableList(),
//            id = id
//        )
//    }
} 