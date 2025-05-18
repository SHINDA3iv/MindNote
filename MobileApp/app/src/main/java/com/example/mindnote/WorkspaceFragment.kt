package com.example.mindnote

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.util.Log
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.fragment.findNavController
import com.example.mindnote.data.*
import java.io.File

class WorkspaceFragment : Fragment() {
    private lateinit var workspaceName: String
    private lateinit var container: LinearLayout
    private lateinit var viewModel: MainViewModel
    private var currentWorkspace: Workspace? = null

    companion object {
        private const val PICK_IMAGE_REQUEST = 1
        private const val PICK_FILE_REQUEST = 2
        
        fun newInstance(workspaceName: String): WorkspaceFragment {
            val fragment = WorkspaceFragment()
            val args = Bundle()
            args.putString("WORKSPACE_NAME", workspaceName)
            fragment.arguments = args
            return fragment
        }
    }

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
        currentWorkspace = viewModel.getWorkspaceByName(workspaceName)
        currentWorkspace?.let { workspace ->
            viewModel.setCurrentWorkspace(workspace)
            loadWorkspaceContent(workspace)
        }

        // Observe current workspace changes
        viewModel.currentWorkspace.observe(viewLifecycleOwner) { workspace ->
            if (workspace.name == workspaceName) {
                currentWorkspace = workspace
                loadWorkspaceContent(workspace)
                Log.d("MindNote", "WorkspaceFragment: Observed workspace '${workspace.name}' with ${workspace.items.size} items")
            }
        }

        viewModel.workspaces.observe(viewLifecycleOwner) { workspaces ->
            val updatedWorkspace = workspaces.find { it.name == workspaceName }
            if (updatedWorkspace != null && (currentWorkspace == null || updatedWorkspace.items.size != currentWorkspace?.items?.size)) {
                currentWorkspace = updatedWorkspace
                loadWorkspaceContent(updatedWorkspace)
                Log.d("MindNote", "WorkspaceFragment: Observed workspaces update, '${updatedWorkspace.name}' now has ${updatedWorkspace.items.size} items")
            }
        }

        return view
    }

    override fun onResume() {
        super.onResume()
        Log.d("MindNote", "WorkspaceFragment: onResume for workspace '${currentWorkspace?.name}'")
        // Обновляем текущее рабочее пространство из хранилища
        currentWorkspace?.let { workspace ->
            currentWorkspace = viewModel.getWorkspaceByName(workspace.name)
            currentWorkspace?.let { 
                loadWorkspaceContent(it)
            }
        }
    }

    private fun loadWorkspaceContent(workspace: Workspace) {
        Log.d("MindNote", "WorkspaceFragment: Loading content for '${workspace.name}', ${workspace.items.size} items")
        container.removeAllViews()
        workspace.items.forEach { item ->
            when (item) {
                is ContentItem.TextItem -> addTextField(item)
                is ContentItem.CheckboxItem -> addCheckboxItem(item)
                is ContentItem.NumberedListItem -> addNumberedListItem(item)
                is ContentItem.BulletListItem -> addBulletListItem(item)
                is ContentItem.ImageItem -> addImageView(item)
                is ContentItem.FileItem -> addFileItem(item)
                is ContentItem.SubWorkspaceLink -> addSubWorkspaceLink(item)
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
                    currentWorkspace?.let { 
                        viewModel.updateContentItem(it, item)
                        viewModel.saveWorkspaces()
                        Log.d("MindNote", "Обновлен текстовый элемент в '${it.name}'")
                    }
                } else {
                    val newItem = ContentItem.TextItem(text)
                    currentWorkspace?.let { 
                        viewModel.addContentItem(it, newItem)
                        viewModel.saveWorkspaces()
                        Log.d("MindNote", "Добавлен новый текстовый элемент в '${it.name}'")
                    }
                }
            }
        }
        
        // Добавляем дополнительное событие для сохранения при вводе текста
        editText.addTextChangedListener(object : android.text.TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
            override fun afterTextChanged(s: android.text.Editable?) {
                s?.toString()?.let { text ->
                    if (item != null) {
                        item.text = text
                        // Обновляем, но не сохраняем при каждом изменении текста
                        currentWorkspace?.let { 
                            viewModel.updateContentItem(it, item)
                        }
                    }
                }
            }
        })
        
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
                currentWorkspace?.let { 
                    viewModel.updateContentItem(it, item)
                    viewModel.saveWorkspaces()
                }
            }
        }

        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                val text = editText.text.toString()
                if (item != null) {
                    item.text = text
                    currentWorkspace?.let { 
                        viewModel.updateContentItem(it, item)
                        viewModel.saveWorkspaces()
                    }
                } else {
                    val newItem = ContentItem.CheckboxItem(text, checkbox.isChecked)
                    currentWorkspace?.let { 
                        viewModel.addContentItem(it, newItem)
                        viewModel.saveWorkspaces()
                    }
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
                    currentWorkspace?.let { 
                        viewModel.updateContentItem(it, item)
                        viewModel.saveWorkspaces()
                    }
                } else {
                    val newItem = ContentItem.NumberedListItem(text, number.toInt())
                    currentWorkspace?.let { 
                        viewModel.addContentItem(it, newItem)
                        viewModel.saveWorkspaces()
                    }
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
                    currentWorkspace?.let { 
                        viewModel.updateContentItem(it, item)
                        viewModel.saveWorkspaces()
                    }
                } else {
                    val newItem = ContentItem.BulletListItem(text)
                    currentWorkspace?.let { 
                        viewModel.addContentItem(it, newItem)
                        viewModel.saveWorkspaces()
                    }
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
        imageView.adjustViewBounds = true
        imageView.scaleType = ImageView.ScaleType.FIT_CENTER

        item?.let {
            try {
                imageView.setImageURI(it.imageUri)
                Log.d("MindNote", "Set image URI: ${it.imageUri}")
            } catch (e: Exception) {
                Log.e("MindNote", "Failed to set image URI: ${it.imageUri}", e)
                // Если не удалось загрузить изображение, устанавливаем placeholder
                imageView.setImageResource(android.R.drawable.ic_menu_report_image)
            }

            if (currentWorkspace != null && item.id.isNotEmpty()) {
                viewModel.updateContentItem(currentWorkspace!!, item)
                viewModel.saveWorkspaces()
                Log.d("MindNote", "Добавлено изображение в '${currentWorkspace!!.name}'")
            }
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

            if (currentWorkspace != null && item.id.isNotEmpty()) {
                viewModel.updateContentItem(currentWorkspace!!, item)
            }
        }

        fileItemView.setOnClickListener {
            item?.let { fileItem ->
                try {
                    val intent = Intent(Intent.ACTION_VIEW).apply {
                        setDataAndType(fileItem.fileUri, getMimeType(fileItem.fileName))
                        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                    }
                    startActivity(intent)
                } catch (e: Exception) {
                    Log.e("MindNote", "Failed to open file: ${fileItem.fileUri}", e)
                    Toast.makeText(context, "Не удалось открыть файл", Toast.LENGTH_SHORT).show()
                }
            }
        }

        fileItemView.setOnLongClickListener {
            showPopupMenuForItem(fileItemView, item)
            true
        }

        container.addView(fileItemView)
    }

    private fun showPopupMenuForItem(view: View, item: ContentItem?) {
        val popupMenu = PopupMenu(context, view)
        popupMenu.menuInflater.inflate(R.menu.popup_menu, popupMenu.menu)
        
        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.popup_option1 -> {
                    // Добавить текстовое поле
                    addTextField()
                    true
                }
                R.id.popup_option2 -> {
                    // Добавить чекбокс
                    addCheckboxItem()
                    true
                }
                R.id.popup_option3 -> {
                    // Добавить изображение
                    val intent = Intent(Intent.ACTION_PICK)
                    intent.type = "image/*"
                    startActivityForResult(intent, PICK_IMAGE_REQUEST)
                    true
                }
                R.id.popup_option4 -> {
                    // Добавить файл
                    val intent = Intent(Intent.ACTION_GET_CONTENT)
                    intent.type = "*/*"
                    intent.addCategory(Intent.CATEGORY_OPENABLE)
                    startActivityForResult(intent, PICK_FILE_REQUEST)
                    true
                }
                R.id.popup_option5 -> {
                    // Добавить нумерованный список
                    addNumberedListItem()
                    true
                }
                R.id.popup_option6 -> {
                    // Добавить маркированный список
                    addBulletListItem()
                    true
                }
                R.id.popup_option7 -> {
                    // Добавить ссылку на подпространство
                    addSubWorkspaceLink()
                    true
                }
                else -> false
            }
        }
        popupMenu.show()
    }

    private fun addSubWorkspaceLink(item: ContentItem.SubWorkspaceLink? = null) {
        val linkView = layoutInflater.inflate(R.layout.subworkspace_link_item, null)
        val editText = linkView.findViewById<EditText>(R.id.editTextLink)
        val workspaceId = item?.workspaceId ?: ""
        val displayName = item?.displayName ?: ""

        editText.setText(displayName)

        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                val text = editText.text.toString()
                if (item != null) {
                    // Create a new item with updated display name
                    val updatedItem = ContentItem.SubWorkspaceLink(
                        workspaceId = item.workspaceId,
                        displayName = text,
                        id = item.id
                    )
                    currentWorkspace?.let { workspace -> 
                        viewModel.updateContentItem(workspace, updatedItem)
                        viewModel.saveWorkspaces()
                    }
                } else {
                    val newItem = ContentItem.SubWorkspaceLink(workspaceId, text)
                    currentWorkspace?.let { workspace -> 
                        viewModel.addContentItem(workspace, newItem)
                        viewModel.saveWorkspaces()
                    }
                }
            }
        }

        linkView.setOnClickListener {
            // Navigate to the linked workspace
            val workspace = viewModel.getWorkspaceByName(workspaceId)
            workspace?.let { ws ->
                val bundle = Bundle().apply {
                    putString("workspaceId", ws.id)
                }
                findNavController().navigate(R.id.workspaceDetailFragment, bundle)
            }
        }

        linkView.setOnLongClickListener {
            showPopupMenuForItem(linkView, item)
            true
        }

        container.addView(linkView)
    }

    private fun formatFileSize(size: Long): String {
        return when {
            size < 1024 -> "$size B"
            size < 1024 * 1024 -> "${size / 1024} KB"
            size < 1024 * 1024 * 1024 -> "${size / (1024 * 1024)} MB"
            else -> "${size / (1024 * 1024 * 1024)} GB"
        }
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

    override fun onPause() {
        super.onPause()
        Log.d("MindNote", "WorkspaceFragment: onPause for workspace '${currentWorkspace?.name}'")
        // Сохраняем состояние текущего рабочего пространства
        currentWorkspace?.let { workspace ->
            viewModel.saveWorkspaces()
        }
    }

    override fun onStop() {
        super.onStop()
        Log.d("MindNote", "WorkspaceFragment: onStop for workspace '${currentWorkspace?.name}'")
        // Сохраняем состояние текущего рабочего пространства
        currentWorkspace?.let { workspace ->
            viewModel.saveWorkspaces()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d("MindNote", "WorkspaceFragment: onDestroy for workspace '${currentWorkspace?.name}'")
        // Сохраняем состояние текущего рабочего пространства
        currentWorkspace?.let { workspace ->
            viewModel.saveWorkspaces()
        }
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        
        // Настраиваем таймер автосохранения
        val handler = Handler(android.os.Looper.getMainLooper())
        val runnable = object : Runnable {
            override fun run() {
                forceContentSave()
                handler.postDelayed(this, 10000) // автосохранение каждые 10 секунд
            }
        }
        handler.postDelayed(runnable, 10000)
    }

    // Общий метод для принудительного сохранения содержимого
    private fun forceContentSave() {
        currentWorkspace?.let { workspace ->
            viewModel.saveWorkspaces()
            Log.d("MindNote", "Принудительное сохранение рабочего пространства '${workspace.name}'")
        }
    }
}
