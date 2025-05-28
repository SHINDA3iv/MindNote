package com.example.mindnote.views

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.view.View
import android.widget.ImageButton
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.PopupMenu
import android.widget.Toast
import android.app.AlertDialog
import android.widget.Button
import android.util.Log
import com.example.mindnote.MainActivity
import com.example.mindnote.R
import com.example.mindnote.data.Workspace
import android.widget.ImageView

class ExpandableMenuItem @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : LinearLayout(context, attrs, defStyleAttr) {

    private var workspace: Workspace? = null
    private var onItemClickListener: ((Workspace) -> Unit)? = null
    private var onWorkspaceEdited: ((Workspace) -> Unit)? = null
    private var onWorkspaceDeleted: ((Workspace) -> Unit)? = null
    private var isExpanded = false

    private val expandButton: ImageButton
    private val titleView: TextView
    private val nestedItemsContainer: LinearLayout
    private val iconView: ImageView

    init {
        orientation = VERTICAL
        LayoutInflater.from(context).inflate(R.layout.expandable_menu_item, this, true)

        expandButton = findViewById(R.id.expand_button)
        titleView = findViewById(R.id.menu_item_title)
        nestedItemsContainer = findViewById(R.id.nested_items_container)
        iconView = findViewById(R.id.menu_item_icon)

        expandButton.setOnClickListener {
            toggleExpansion()
        }

        setOnClickListener {
            workspace?.let { ws ->
                onItemClickListener?.invoke(ws)
            }
        }

        setOnLongClickListener { view ->
            workspace?.let { ws ->
                showContextMenu(view, ws)
                true
            } ?: false
        }
    }

    fun setWorkspace(workspace: Workspace) {
        this.workspace = workspace
        updateUI()
    }

    fun setOnItemClickListener(listener: (Workspace) -> Unit) {
        onItemClickListener = listener
    }

    fun setOnWorkspaceEdited(listener: (Workspace) -> Unit) {
        onWorkspaceEdited = listener
    }

    fun setOnWorkspaceDeleted(listener: (Workspace) -> Unit) {
        onWorkspaceDeleted = listener
    }

    private fun updateUI() {
        workspace?.let { ws ->
            titleView.text = ws.name
            ws.iconUri?.let { uri ->
                try {
                    val inputStream = context.contentResolver.openInputStream(uri)
                    val bitmap = android.graphics.BitmapFactory.decodeStream(inputStream)
                    iconView.setImageBitmap(bitmap)
                } catch (e: Exception) {
                    Log.e("MindNote", "Error loading icon", e)
                    iconView.setImageResource(R.drawable.ic_workspace)
                }
            } ?: run {
                iconView.setImageResource(R.drawable.ic_workspace)
            }
            updateNestedItems()
        }
    }

    private fun toggleExpansion() {
        isExpanded = !isExpanded
        expandButton.rotation = if (isExpanded) 180f else 0f
        nestedItemsContainer.visibility = if (isExpanded) View.VISIBLE else View.GONE
    }

    private fun updateNestedItems() {
        nestedItemsContainer.removeAllViews()
        workspace?.let { ws ->
            val mainActivity = context as? MainActivity
            if (mainActivity == null) {
                Log.e("MindNote", "Context is not MainActivity")
                return
            }

            ws.items.forEach { item ->
                if (item is com.example.mindnote.data.ContentItem.NestedPageItem) {
                    val nestedItem = ExpandableMenuItem(context)
                    val nestedWorkspace = mainActivity.getWorkspaceById(item.pageId)
                    nestedWorkspace?.let { nestedWs ->
                        if (nestedWs.iconUri == null) {
                            nestedWs.iconUri = mainActivity.getDefaultWorkspaceIconUri()
                        }
                        nestedItem.setWorkspace(nestedWs)
                        nestedItem.setOnItemClickListener { nestedWs ->
                            onItemClickListener?.invoke(nestedWs)
                        }
                        nestedItem.setOnWorkspaceEdited { nestedWs ->
                            onWorkspaceEdited?.invoke(nestedWs)
                        }
                        nestedItem.setOnWorkspaceDeleted { nestedWs ->
                            onWorkspaceDeleted?.invoke(nestedWs)
                        }
                        nestedItemsContainer.addView(nestedItem)
                    }
                }
            }
        }
    }

    private fun showContextMenu(view: View, workspace: Workspace) {
        try {
            val popupMenu = PopupMenu(context, view)
            popupMenu.menuInflater.inflate(R.menu.workspace_context_menu, popupMenu.menu)

            popupMenu.setOnMenuItemClickListener { menuItem ->
                when (menuItem.itemId) {
                    R.id.menu_rename -> {
                        showEditDialog(workspace)
                        true
                    }
                    R.id.menu_change_icon -> {
                        val mainActivity = context as? MainActivity
                        if (mainActivity != null) {
                            mainActivity.showIconSelectionDialog { uri ->
                                uri?.let {
                                    workspace.iconUri = it
                                    onWorkspaceEdited?.invoke(workspace)
                                }
                            }
                            true
                        } else {
                            Log.e("MindNote", "Context is not MainActivity")
                            false
                        }
                    }
                    R.id.menu_delete -> {
                        showDeleteConfirmation(workspace)
                        true
                    }
                    else -> false
                }
            }
            popupMenu.show()
        } catch (e: Exception) {
            Log.e("MindNote", "Error showing context menu", e)
        }
    }

    private fun showEditDialog(workspace: Workspace) {
        val mainActivity = context as? MainActivity
        mainActivity?.let { activity ->
            val dialogView = LayoutInflater.from(context).inflate(R.layout.dialog_add_menu_item, null)
            val editText = dialogView.findViewById<TextView>(R.id.editTextMenuItem)
            val iconButton = dialogView.findViewById<Button>(R.id.buttonSelectIcon)

            editText.setText(workspace.name)
            var selectedIconUri = workspace.iconUri

            iconButton.setOnClickListener {
                activity.showIconSelectionDialog { uri ->
                    selectedIconUri = uri
                    uri?.let { previewUri ->
                        try {
                            val inputStream = context.contentResolver.openInputStream(previewUri)
                            val bitmap = android.graphics.BitmapFactory.decodeStream(inputStream)
                            iconButton.setCompoundDrawablesWithIntrinsicBounds(
                                android.graphics.drawable.BitmapDrawable(resources, bitmap),
                                null, null, null
                            )
                        } catch (e: Exception) {
                            Log.e("MindNote", "Error loading icon preview", e)
                        }
                    }
                }
            }

            AlertDialog.Builder(context)
                .setTitle("Редактировать пространство")
                .setView(dialogView)
                .setPositiveButton("Сохранить") { _, _ ->
                    val newName = editText.text.toString().trim()
                    if (newName.isNotEmpty()) {
                        workspace.name = newName
                        workspace.iconUri = selectedIconUri
                        onWorkspaceEdited?.invoke(workspace)
                    } else {
                        Toast.makeText(context, "Имя не может быть пустым", Toast.LENGTH_SHORT).show()
                    }
                }
                .setNegativeButton("Отмена", null)
                .show()
        }
    }

    private fun showDeleteConfirmation(workspace: Workspace) {
        AlertDialog.Builder(context)
            .setTitle("Удалить пространство")
            .setMessage("Вы уверены, что хотите удалить пространство '${workspace.name}'?")
            .setPositiveButton("Удалить") { _, _ ->
                onWorkspaceDeleted?.invoke(workspace)
            }
            .setNegativeButton("Отмена", null)
            .show()
    }
} 