package com.example.mindnote.network

import com.example.mindnote.data.entity.Page
import com.example.mindnote.data.entity.PageElement
import com.example.mindnote.data.entity.Workspace

data class SyncData(
    val lastSyncTimestamp: Long,
    val workspaces: List<WorkspaceSync>,
    val pages: List<PageSync>,
    val elements: List<ElementSync>
)

data class SyncResponse(
    val timestamp: Long,
    val workspaces: List<WorkspaceSync>,
    val pages: List<PageSync>,
    val elements: List<ElementSync>
)

data class WorkspaceSync(
    val id: Long,
    val name: String,
    val createdAt: Long,
    val isDeleted: Boolean = false,
    val serverTimestamp: Long? = null
)

data class PageSync(
    val id: Long,
    val workspaceId: Long,
    val title: String,
    val position: Int,
    val createdAt: Long,
    val isDeleted: Boolean = false,
    val serverTimestamp: Long? = null
)

data class ElementSync(
    val id: Long,
    val pageId: Long,
    val type: String,
    val content: String,
    val x: Int,
    val y: Int,
    val width: Int,
    val height: Int,
    val createdAt: Long,
    val isDeleted: Boolean = false,
    val serverTimestamp: Long? = null
) 