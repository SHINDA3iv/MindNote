package com.example.mindnote.data

import android.net.Uri
import java.util.UUID

sealed class ContentItem {
    abstract val id: String

    data class TextItem(
        var text: String,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()

    data class CheckboxItem(
        var text: String,
        var isChecked: Boolean = false,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()

    data class NumberedListItem(
        var text: String,
        var number: Int,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()

    data class BulletListItem(
        var text: String,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()

    data class ImageItem(
        var imageUri: Uri,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()

    data class FileItem(
        var fileName: String,
        var fileUri: Uri,
        var fileSize: Long,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()

    // Новый тип: ссылка на вложенное пространство
    data class SubWorkspaceLink(
        val workspaceId: String,
        val displayName: String,
        override val id: String = UUID.randomUUID().toString()
    ) : ContentItem()
} 