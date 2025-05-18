package com.example.mindnote.data

import android.net.Uri
import java.util.UUID

// Модель рабочего пространства с поддержкой вложенности

data class Workspace(
    var id: String = UUID.randomUUID().toString(),
    var name: String,
    var iconUri: Uri? = null,
    var parentId: String? = null, // id родительского пространства, если есть
    var children: MutableList<Workspace> = mutableListOf(), // вложенные пространства
    private val _items: MutableList<ContentItem> = mutableListOf(),
    var lastAccessed: Long = System.currentTimeMillis(), // время последнего доступа
    var isFavorite: Boolean = false // флаг избранного
) {
    val items: List<ContentItem> get() = _items.toList()

    // Добавить элемент
    fun addItem(item: ContentItem) {
        _items.add(item)
    }

    // Удалить элемент
    fun removeItem(itemId: String) {
        _items.removeAll { it.id == itemId }
    }

    // Обновить элемент
    fun updateItem(updatedItem: ContentItem) {
        val idx = _items.indexOfFirst { it.id == updatedItem.id }
        if (idx != -1) _items[idx] = updatedItem
    }

    // Добавить подпространство (если имя уникально среди детей)
    fun addChild(child: Workspace): Boolean {
        if (children.any { it.name == child.name }) return false
        child.parentId = this.id
        children.add(child)
        return true
    }

    // Удалить подпространство по id
    fun removeChild(childId: String) {
        children.removeAll { it.id == childId }
    }

    // Найти подпространство по id (рекурсивно)
    fun findChildById(childId: String): Workspace? {
        if (this.id == childId) return this
        for (child in children) {
            val found = child.findChildById(childId)
            if (found != null) return found
        }
        return null
    }

    // Проверить уникальность имени среди детей
    fun isChildNameUnique(name: String): Boolean {
        return children.none { it.name == name }
    }
} 