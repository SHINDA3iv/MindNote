package com.example.mindnote

import android.app.ProgressDialog
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
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
import androidx.lifecycle.lifecycleScope
import com.example.mindnote.data.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.util.concurrent.ConcurrentHashMap

class WorkspaceFragment : Fragment() {
    private lateinit var workspaceName: String
    private lateinit var container: LinearLayout
    private lateinit var viewModel: MainViewModel
    private var currentWorkspace: Workspace? = null
    private val imageCache = ConcurrentHashMap<String, Bitmap>()
    private var isDestroyed = false

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
            val updatedWorkspace = viewModel.getWorkspaceByName(workspace.name)
            if (updatedWorkspace != null) {
                currentWorkspace = updatedWorkspace
                loadWorkspaceContent(updatedWorkspace)
                Log.d("MindNote", "WorkspaceFragment: Reloaded workspace '${updatedWorkspace.name}' with ${updatedWorkspace.items.size} items")
            }
        }
    }

    private fun loadWorkspaceContent(workspace: Workspace) {
        Log.d("MindNote", "WorkspaceFragment: Loading content for '${workspace.name}', ${workspace.items.size} items")
        
        // Очищаем контейнер перед загрузкой
        container.removeAllViews()
        
        // Сначала загружаем все элементы кроме изображений
        workspace.items.forEach { item ->
            when (item) {
                is ContentItem.TextItem -> addTextField(item)
                is ContentItem.CheckboxItem -> addCheckboxItem(item)
                is ContentItem.NumberedListItem -> addNumberedListItem(item)
                is ContentItem.BulletListItem -> addBulletListItem(item)
                is ContentItem.FileItem -> addFileItem(item)
                is ContentItem.ImageItem -> {
                    // Для изображений создаем заглушку
                    val placeholder = ImageView(context).apply {
                        layoutParams = LinearLayout.LayoutParams(
                            LinearLayout.LayoutParams.MATCH_PARENT,
                            LinearLayout.LayoutParams.WRAP_CONTENT
                        )
                        setImageResource(android.R.drawable.ic_menu_report_image)
                    }
                    container.addView(placeholder)
                }
            }
        }
        
        // Затем асинхронно загружаем изображения
        viewLifecycleOwner.lifecycleScope.launch(Dispatchers.IO) {
            workspace.items.forEach { item ->
                if (isDestroyed) return@forEach
                
                if (item is ContentItem.ImageItem) {
                    try {
                        val bitmap = loadImage(item.imageUri.toString())
                        if (bitmap != null && !isDestroyed) {
                            withContext(Dispatchers.Main) {
                                val index = workspace.items.indexOf(item)
                                if (index >= 0 && index < container.childCount) {
                                    container.removeViewAt(index)
                                    addImageView(item, bitmap)
                                }
                            }
                        }
                    } catch (e: Exception) {
                        Log.e("MindNote", "Failed to load image: ${item.imageUri}", e)
                    }
                    // Добавляем задержку между загрузкой изображений
                    delay(200)
                }
            }
        }
    }

    private suspend fun loadImage(uriString: String): Bitmap? = withContext(Dispatchers.IO) {
        try {
            // Проверяем кэш
            imageCache[uriString]?.let { return@withContext it }

            val uri = Uri.parse(uriString)
            context?.contentResolver?.openInputStream(uri)?.use { input ->
                // Сначала получаем размеры изображения
                val options = BitmapFactory.Options().apply {
                    inJustDecodeBounds = true
                }
                BitmapFactory.decodeStream(input, null, options)

                // Вычисляем коэффициент масштабирования
                val maxWidth = resources.displayMetrics.widthPixels
                val maxHeight = resources.displayMetrics.heightPixels
                val widthRatio = options.outWidth.toFloat() / maxWidth
                val heightRatio = options.outHeight.toFloat() / maxHeight
                val sampleSize = if (widthRatio > 1 || heightRatio > 1) {
                    maxOf(widthRatio, heightRatio).toInt()
                } else {
                    1
                }

                // Загружаем изображение с учетом масштабирования
                val loadOptions = BitmapFactory.Options().apply {
                    inSampleSize = sampleSize
                }

                context?.contentResolver?.openInputStream(uri)?.use { input ->
                    val bitmap = BitmapFactory.decodeStream(input, null, loadOptions)
                    if (bitmap != null) {
                        // Сохраняем в кэш
                        imageCache[uriString] = bitmap
                    }
                    bitmap
                }
            }
        } catch (e: Exception) {
            Log.e("MindNote", "Error loading image: $uriString", e)
            null
        }
    }

    fun addTextField(item: ContentItem.TextItem? = null) {
        val editText = EditText(context)
        editText.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        item?.let { editText.setText(it.text) }
        
        // Добавляем TextWatcher для сохранения при вводе текста
        editText.addTextChangedListener(object : android.text.TextWatcher {
            private var lastSavedText = editText.text.toString()
            private var saveHandler = Handler(android.os.Looper.getMainLooper())
            private var saveRunnable: Runnable? = null
            
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
            override fun afterTextChanged(s: android.text.Editable?) {
                s?.toString()?.let { text ->
                    if (text != lastSavedText) {
                        // Отменяем предыдущее сохранение, если оно было запланировано
                        saveRunnable?.let { saveHandler.removeCallbacks(it) }
                        
                        // Создаем новое сохранение с задержкой
                        saveRunnable = Runnable {
                            lastSavedText = text
                            if (item != null) {
                                item.text = text
                                currentWorkspace?.let { 
                                    viewModel.updateContentItem(it, item)
                                    viewModel.saveWorkspaces()
                                }
                            } else {
                                val newItem = ContentItem.TextItem(text)
                                currentWorkspace?.let { 
                                    viewModel.addContentItem(it, newItem)
                                    viewModel.saveWorkspaces()
                                }
                            }
                        }
                        // Запускаем сохранение с задержкой в 500 мс
                        saveHandler.postDelayed(saveRunnable!!, 500)
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

        // Сохраняем ссылку на текущий элемент
        var currentItem: ContentItem.CheckboxItem? = item

        item?.let {
            editText.setText(it.text)
            checkbox.isChecked = it.isChecked
            Log.d("MindNote", "Loading checkbox item: id=${it.id}, text='${it.text}', isChecked=${it.isChecked}")
        }

        checkbox.setOnCheckedChangeListener { _, isChecked ->
            Log.d("MindNote", "Checkbox state changed: isChecked=$isChecked")
            currentItem?.let { checkboxItem ->
                // Создаем новый элемент с обновленным состоянием
                val updatedItem = checkboxItem.copy(isChecked = isChecked)
                currentWorkspace?.let { workspace -> 
                    // Обновляем элемент в рабочем пространстве
                    viewModel.updateContentItem(workspace, updatedItem)
                    // Обновляем ссылку на текущий элемент
                    currentItem = updatedItem
                    // Принудительно сохраняем изменения
                    viewModel.saveWorkspaces()
                    Log.d("MindNote", "Updated checkbox state in workspace '${workspace.name}': id=${updatedItem.id}")
                }
            } ?: run {
                // Если это новый элемент, создаем его
                val text = editText.text.toString()
                val newItem = ContentItem.CheckboxItem(text, isChecked)
                currentWorkspace?.let { workspace ->
                    viewModel.addContentItem(workspace, newItem)
                    currentItem = newItem // Обновляем ссылку на текущий элемент
                    // Принудительно сохраняем изменения
                    viewModel.saveWorkspaces()
                    Log.d("MindNote", "Added new checkbox item to workspace '${workspace.name}': id=${newItem.id}")
                }
            }
        }

        // Добавляем TextWatcher для сохранения текста чекбокса
        editText.addTextChangedListener(object : android.text.TextWatcher {
            private var lastSavedText = editText.text.toString()
            private var saveHandler = Handler(android.os.Looper.getMainLooper())
            private var saveRunnable: Runnable? = null
            
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
            override fun afterTextChanged(s: android.text.Editable?) {
                s?.toString()?.let { text ->
                    if (text != lastSavedText) {
                        // Отменяем предыдущее сохранение, если оно было запланировано
                        saveRunnable?.let { saveHandler.removeCallbacks(it) }
                        
                        // Создаем новое сохранение с задержкой
                        saveRunnable = Runnable {
                            lastSavedText = text
                            currentItem?.let { checkboxItem ->
                                // Создаем новый элемент с обновленным текстом и текущим состоянием чекбокса
                                val updatedItem = checkboxItem.copy(
                                    text = text,
                                    isChecked = checkbox.isChecked
                                )
                                currentWorkspace?.let { workspace -> 
                                    viewModel.updateContentItem(workspace, updatedItem)
                                    // Обновляем ссылку на текущий элемент
                                    currentItem = updatedItem
                                    // Принудительно сохраняем изменения
                                    viewModel.saveWorkspaces()
                                    Log.d("MindNote", "Updated checkbox text in workspace '${workspace.name}': id=${updatedItem.id}, isChecked=${checkbox.isChecked}")
                                }
                            } ?: run {
                                val newItem = ContentItem.CheckboxItem(text, checkbox.isChecked)
                                currentWorkspace?.let { workspace -> 
                                    viewModel.addContentItem(workspace, newItem)
                                    currentItem = newItem // Обновляем ссылку на текущий элемент
                                    // Принудительно сохраняем изменения
                                    viewModel.saveWorkspaces()
                                    Log.d("MindNote", "Added new checkbox item with text to workspace '${workspace.name}': id=${newItem.id}, isChecked=${checkbox.isChecked}")
                                }
                            }
                        }
                        // Запускаем сохранение с задержкой в 500 мс
                        saveHandler.postDelayed(saveRunnable!!, 500)
                    }
                }
            }
        })

        checkboxItemView.setOnLongClickListener {
            showPopupMenuForItem(checkboxItemView, currentItem)
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

    fun addImageView(item: ContentItem.ImageItem? = null, preloadedBitmap: Bitmap? = null) {
        val imageView = ImageView(context)
        imageView.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        imageView.adjustViewBounds = true
        imageView.scaleType = ImageView.ScaleType.FIT_CENTER

        item?.let {
            try {
                if (preloadedBitmap != null) {
                    imageView.setImageBitmap(preloadedBitmap)
                } else {
                    // Загружаем изображение в фоновом потоке
                    viewLifecycleOwner.lifecycleScope.launch(Dispatchers.IO) {
                        try {
                            val bitmap = loadImage(it.imageUri.toString())
                            if (bitmap != null && !isDestroyed) {
                                withContext(Dispatchers.Main) {
                                    imageView.setImageBitmap(bitmap)
                                }
                            }
                        } catch (e: Exception) {
                            Log.e("MindNote", "Failed to load image: ${it.imageUri}", e)
                            withContext(Dispatchers.Main) {
                                imageView.setImageResource(android.R.drawable.ic_menu_report_image)
                            }
                        }
                    }
                }

                if (currentWorkspace != null && item.id.isNotEmpty()) {
                    viewModel.updateContentItem(currentWorkspace!!, item)
                    viewModel.saveWorkspaces()
                }
            } catch (e: Exception) {
                Log.e("MindNote", "Failed to set image URI: ${it.imageUri}", e)
                imageView.setImageResource(android.R.drawable.ic_menu_report_image)
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
        popupMenu.menuInflater.inflate(R.menu.item_popup_menu, popupMenu.menu)

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
                            viewModel.saveWorkspaces()
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
        isDestroyed = true
        // Очищаем кэш изображений
        imageCache.clear()
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
