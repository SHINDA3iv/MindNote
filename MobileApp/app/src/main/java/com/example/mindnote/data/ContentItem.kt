package com.example.mindnote.data

import android.net.Uri

sealed class ContentItem {
    abstract val id: String

    data class TextItem(
        var text: String,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class CheckboxItem(
        var text: String,
        var isChecked: Boolean = false,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class ImageItem(
        var imageUri: Uri,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class FileItem(
        var fileName: String,
        var fileUri: Uri,
        var fileSize: Long,
        override val id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()
} 