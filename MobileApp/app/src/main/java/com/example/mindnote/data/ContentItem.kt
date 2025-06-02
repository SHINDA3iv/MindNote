package com.example.mindnote.data

import android.net.Uri

sealed class ContentItem {
    abstract val id: String

    data class TextItem(
        var text: String,
        var htmlText: String = text,
        var isFormatted: Boolean = false,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class CheckboxItem(
        var text: String,
        var htmlText: String = text,
        var isFormatted: Boolean = false,
        var isChecked: Boolean = false,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class ImageItem(
        val imageUri: Uri,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class FileItem(
        val fileName: String,
        val fileUri: Uri,
        val fileSize: Long,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class NestedPageItem(
        var pageName: String,
        val pageId: String,
        var iconUri: Uri? = null,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()
} 