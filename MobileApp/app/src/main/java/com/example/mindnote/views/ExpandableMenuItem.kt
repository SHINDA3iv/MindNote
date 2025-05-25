package com.example.mindnote.views

import android.content.Context
import android.net.Uri
import android.util.AttributeSet
import android.view.LayoutInflater
import android.view.View
import android.widget.ImageButton
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import com.example.mindnote.MainActivity
import com.example.mindnote.R
import com.example.mindnote.data.Workspace

class ExpandableMenuItem @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : LinearLayout(context, attrs, defStyleAttr) {

    private val headerView: LinearLayout
    private val iconView: ImageView
    private val titleView: TextView
    private val expandButton: ImageButton
    private val nestedContainer: LinearLayout
    private var isExpanded = false
    private var workspace: Workspace? = null
    private var onItemClickListener: ((Workspace) -> Unit)? = null

    init {
        orientation = VERTICAL
        layoutParams = LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT)
        LayoutInflater.from(context).inflate(R.layout.expandable_menu_item, this, true)

        headerView = findViewById(R.id.menu_item_header)
        iconView = findViewById(R.id.menu_item_icon)
        titleView = findViewById(R.id.menu_item_title)
        expandButton = findViewById(R.id.expand_button)
        nestedContainer = findViewById(R.id.nested_items_container)

        // Устанавливаем параметры для headerView
        headerView.layoutParams = LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT)

        setupClickListeners()
    }

    private fun setupClickListeners() {
        headerView.setOnClickListener {
            workspace?.let { ws ->
                if (ws.items.any { it is com.example.mindnote.data.ContentItem.NestedPageItem }) {
                    toggleExpansion()
                } else {
                    onItemClickListener?.invoke(ws)
                }
            }
        }

        expandButton.setOnClickListener {
            toggleExpansion()
        }
    }

    private fun toggleExpansion() {
        isExpanded = !isExpanded
        expandButton.rotation = if (isExpanded) 180f else 0f
        nestedContainer.visibility = if (isExpanded) View.VISIBLE else View.GONE
    }

    fun setWorkspace(workspace: Workspace) {
        this.workspace = workspace
        titleView.text = workspace.name
        workspace.iconUri?.let { uri ->
            try {
                val inputStream = context.contentResolver.openInputStream(uri)
                val bitmap = android.graphics.BitmapFactory.decodeStream(inputStream)
                iconView.setImageBitmap(bitmap)
            } catch (e: Exception) {
                iconView.setImageResource(android.R.drawable.ic_menu_agenda)
            }
        } ?: run {
            iconView.setImageResource(android.R.drawable.ic_menu_agenda)
        }

        // Add nested items
        nestedContainer.removeAllViews()
        workspace.items.filterIsInstance<com.example.mindnote.data.ContentItem.NestedPageItem>()
            .forEach { nestedItem ->
                addNestedItem(nestedItem)
            }
    }

    private fun addNestedItem(nestedItem: com.example.mindnote.data.ContentItem.NestedPageItem) {
        val mainActivity = context as? MainActivity
        val targetWorkspace = mainActivity?.getWorkspaceById(nestedItem.pageId)
        
        if (targetWorkspace != null && targetWorkspace.items.any { it is com.example.mindnote.data.ContentItem.NestedPageItem }) {
            // Если у вложенной страницы есть свои дочерние страницы, создаем новый ExpandableMenuItem
            val nestedExpandableItem = ExpandableMenuItem(context)
            nestedExpandableItem.setWorkspace(targetWorkspace)
            nestedExpandableItem.setOnItemClickListener { ws ->
                onItemClickListener?.invoke(ws)
            }
            nestedContainer.addView(nestedExpandableItem)
        } else {
            // Если нет дочерних страниц, создаем обычный элемент
            val nestedView = LayoutInflater.from(context).inflate(R.layout.nested_page_item, null)
            val nestedIcon = nestedView.findViewById<ImageView>(R.id.pageIcon)
            val nestedTitle = nestedView.findViewById<TextView>(R.id.pageName)

            nestedTitle.text = nestedItem.pageName
            nestedItem.iconUri?.let { uri ->
                try {
                    val inputStream = context.contentResolver.openInputStream(uri)
                    val bitmap = android.graphics.BitmapFactory.decodeStream(inputStream)
                    nestedIcon.setImageBitmap(bitmap)
                } catch (e: Exception) {
                    nestedIcon.setImageResource(android.R.drawable.ic_menu_agenda)
                }
            } ?: run {
                nestedIcon.setImageResource(android.R.drawable.ic_menu_agenda)
            }

            nestedView.setOnClickListener {
                targetWorkspace?.let { ws ->
                    onItemClickListener?.invoke(ws)
                }
            }

            nestedContainer.addView(nestedView)
        }
    }

    fun setOnItemClickListener(listener: (Workspace) -> Unit) {
        onItemClickListener = listener
    }
} 