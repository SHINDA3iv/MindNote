package com.example.mindnote.data

import android.net.Uri

sealed class ContentItem {
    data class TextItem(
        var text: String,
        var id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class CheckboxItem(
        var text: String,
        var isChecked: Boolean = false,
        var id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class NumberedListItem(
        var text: String,
        var number: Int,
        var id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class BulletListItem(
        var text: String,
        var id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class ImageItem(
        var imageUri: Uri,
        var id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()

    data class FileItem(
        var fileName: String,
        var fileUri: Uri,
        var fileSize: Long,
        var id: String = java.util.UUID.randomUUID().toString()
    ) : ContentItem()
} 