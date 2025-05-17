package com.example.mindnote.data

import android.net.Uri

data class Workspace(
    var name: String,
    var iconUri: Uri? = null,
    var isFavorite: Boolean = false,
    var lastAccessed: Long = System.currentTimeMillis(),
    var items: MutableList<ContentItem> = mutableListOf(),
    var id: String = java.util.UUID.randomUUID().toString()
) 