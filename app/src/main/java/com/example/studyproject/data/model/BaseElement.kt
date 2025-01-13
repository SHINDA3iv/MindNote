package com.example.studyproject.data.model

sealed class BaseElement(val id: String) {
    data class TitleElement(val text: String) : BaseElement(id = "title_$text")
    data class TextElement(val text: String) : BaseElement(id = "text_$text")
    data class NumberedListElement(val items: List<String>) : BaseElement(id = "numbered_list")
    data class UnorderedListElement(val items: List<String>) : BaseElement(id = "unordered_list")
    data class CheckboxElement(val items: List<Pair<String, Boolean>>) : BaseElement(id = "checkbox")
    data class ImageElement(val imageUrl: String) : BaseElement(id = "image_$imageUrl")
    data class FileElement(val filePath: String) : BaseElement(id = "file_$filePath")
    data class SubWorkspaceElement(val name: String, val workspaceId: String) : BaseElement(id = "workspace_$workspaceId")
}
