package com.example.mindnote

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.example.mindnote.data.*
import java.io.File

class WorkspaceFragment : Fragment() {
    private lateinit var workspaceName: String
    private lateinit var container: LinearLayout
    private lateinit var viewModel: MainViewModel
    private var currentWorkspace: Workspace? = null

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        workspaceName = arguments?.getString("WORKSPACE_NAME") ?: "Рабочее пространство"
        viewModel = ViewModelProvider(requireActivity())[MainViewModel::class.java]

        val view = inflater.inflate(R.layout.fragment_workspace, container, false)
        (activity as MainActivity).supportActionBar?.title = workspaceName
        this.container = view.findViewById(R.id.container)

        // Find the current workspace
        currentWorkspace = viewModel.workspaces.value?.find { it.name == workspaceName }
        currentWorkspace?.let { workspace ->
            viewModel.setCurrentWorkspace(workspace)
            loadWorkspaceContent(workspace)
        }

        return view
    }

    private fun loadWorkspaceContent(workspace: Workspace) {
        container.removeAllViews()
        workspace.items.forEach { item ->
            when (item) {
                is ContentItem.TextItem -> addTextField(item)
                is ContentItem.CheckboxItem -> addCheckboxItem(item)
                is ContentItem.NumberedListItem -> addNumberedListItem(item)
                is ContentItem.BulletListItem -> addBulletListItem(item)
                is ContentItem.ImageItem -> addImageView(item)
                is ContentItem.FileItem -> addFileItem(item)
            }
        }
    }

    fun addTextField(item: ContentItem.TextItem? = null) {
        val editText = EditText(context)
        editText.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        item?.let { editText.setText(it.text) }
        
        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                val text = editText.text.toString()
                if (item != null) {
                    item.text = text
                    currentWorkspace?.let { viewModel.updateContentItem(it, item) }
                } else {
                    val newItem = ContentItem.TextItem(text)
                    currentWorkspace?.let { viewModel.addContentItem(it, newItem) }
                }
            }
        }
        
        container.addView(editText)
    }

    fun addCheckboxItem(item: ContentItem.CheckboxItem? = null) {
        val checkboxItemView = layoutInflater.inflate(R.layout.checkbox_item, null)
        val checkbox = checkboxItemView.findViewById<CheckBox>(R.id.checkbox)
        val editText = checkboxItemView.findViewById<EditText>(R.id.editTextCheckbox)

        item?.let {
            editText.setText(it.text)
            checkbox.isChecked = it.isChecked
        }

        checkbox.setOnCheckedChangeListener { _, isChecked ->
            if (item != null) {
                item.isChecked = isChecked
                currentWorkspace?.let { viewModel.updateContentItem(it, item) }
            }
        }

        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                val text = editText.text.toString()
                if (item != null) {
                    item.text = text
                    currentWorkspace?.let { viewModel.updateContentItem(it, item) }
                } else {
                    val newItem = ContentItem.CheckboxItem(text, checkbox.isChecked)
                    currentWorkspace?.let { viewModel.addContentItem(it, newItem) }
                }
            }
        }

        checkboxItemView.setOnLongClickListener {
            showPopupMenuForItem(checkboxItemView, item)
            true
        }

        container.addView(checkboxItemView)
    }

    fun addNumberedListItem(item: ContentItem.NumberedListItem? = null) {
        val listItemView = layoutInflater.inflate(R.layout.numbered_list_item, null)
        val numberText = listItemView.findViewById<TextView>(R.id.numberText)
        val editText = listItemView.findViewById<EditText>(R.id.editTextListItem)

        val number = (container.childCount + 1).toString()
        numberText.text = number

        item?.let {
            editText.setText(it.text)
        }

        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                val text = editText.text.toString()
                if (item != null) {
                    item.text = text
                    currentWorkspace?.let { viewModel.updateContentItem(it, item) }
                } else {
                    val newItem = ContentItem.NumberedListItem(text, number.toInt())
                    currentWorkspace?.let { viewModel.addContentItem(it, newItem) }
                }
            }
        }

        listItemView.setOnLongClickListener {
            showPopupMenuForItem(listItemView, item)
            true
        }

        container.addView(listItemView)
    }

    fun addBulletListItem(item: ContentItem.BulletListItem? = null) {
        val listItemView = layoutInflater.inflate(R.layout.bullet_list_item, null)
        val editText = listItemView.findViewById<EditText>(R.id.editTextListItem)

        item?.let {
            editText.setText(it.text)
        }

        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                val text = editText.text.toString()
                if (item != null) {
                    item.text = text
                    currentWorkspace?.let { viewModel.updateContentItem(it, item) }
                } else {
                    val newItem = ContentItem.BulletListItem(text)
                    currentWorkspace?.let { viewModel.addContentItem(it, newItem) }
                }
            }
        }

        listItemView.setOnLongClickListener {
            showPopupMenuForItem(listItemView, item)
            true
        }

        container.addView(listItemView)
    }

    fun addImageView(item: ContentItem.ImageItem? = null) {
        val imageView = ImageView(context)
        imageView.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )

        item?.let {
            imageView.setImageURI(it.imageUri)
        }

        imageView.setOnLongClickListener {
            showPopupMenuForItem(imageView, item)
            true
        }

        container.addView(imageView)
    }

    fun addFileItem(item: ContentItem.FileItem? = null) {
        val fileItemView = layoutInflater.inflate(R.layout.file_item, null)
        val fileNameText = fileItemView.findViewById<TextView>(R.id.fileNameText)
        val fileSizeText = fileItemView.findViewById<TextView>(R.id.fileSizeText)

        item?.let {
            fileNameText.text = it.fileName
            fileSizeText.text = formatFileSize(it.fileSize)
        }

        fileItemView.setOnClickListener {
            item?.let { fileItem ->
                val intent = Intent(Intent.ACTION_VIEW).apply {
                    setDataAndType(fileItem.fileUri, getMimeType(fileItem.fileName))
                    addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                }
                startActivity(intent)
            }
        }

        fileItemView.setOnLongClickListener {
            showPopupMenuForItem(fileItemView, item)
            true
        }

        container.addView(fileItemView)
    }

    private fun showPopupMenuForItem(view: View, item: ContentItem?) {
        val popupMenu = PopupMenu(requireContext(), view)
        popupMenu.menuInflater.inflate(R.menu.popup_menu_item, popupMenu.menu)

        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.menu_delete -> {
                    item?.let {
                        currentWorkspace?.let { workspace ->
                            val itemId = when (it) {
                                is ContentItem.TextItem -> it.id
                                is ContentItem.CheckboxItem -> it.id
                                is ContentItem.NumberedListItem -> it.id
                                is ContentItem.BulletListItem -> it.id
                                is ContentItem.ImageItem -> it.id
                                is ContentItem.FileItem -> it.id
                            }
                            viewModel.removeContentItem(workspace, itemId)
                            container.removeView(view)
                        }
                    }
                    true
                }
                else -> false
            }
        }

        popupMenu.show()
    }

    private fun formatFileSize(size: Long): String {
        val units = arrayOf("B", "KB", "MB", "GB")
        var value = size.toDouble()
        var unitIndex = 0
        while (value >= 1024 && unitIndex < units.size - 1) {
            value /= 1024
            unitIndex++
        }
        return String.format("%.1f %s", value, units[unitIndex])
    }

    private fun getMimeType(fileName: String): String {
        return when {
            fileName.endsWith(".pdf") -> "application/pdf"
            fileName.endsWith(".doc") || fileName.endsWith(".docx") -> "application/msword"
            fileName.endsWith(".xls") || fileName.endsWith(".xlsx") -> "application/vnd.ms-excel"
            fileName.endsWith(".ppt") || fileName.endsWith(".pptx") -> "application/vnd.ms-powerpoint"
            fileName.endsWith(".txt") -> "text/plain"
            else -> "application/octet-stream"
        }
    }

    companion object {
        fun newInstance(workspaceName: String): WorkspaceFragment {
            val fragment = WorkspaceFragment()
            val args = Bundle()
            args.putString("WORKSPACE_NAME", workspaceName)
            fragment.arguments = args
            return fragment
        }
    }
}
