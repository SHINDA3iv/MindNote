package com.example.mindnote

import android.app.Activity
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.text.Html
import android.text.SpannableStringBuilder
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
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.result.ActivityResultLauncher

class WorkspaceFragment : Fragment() {
    private lateinit var workspaceName: String
    private lateinit var container: LinearLayout
    private lateinit var viewModel: MainViewModel
    private var currentWorkspace: Workspace? = null
    private val imageCache = ConcurrentHashMap<String, Bitmap>()
    private var isDestroyed = false
    private var isContentLoaded = false

    private val pickIconLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            result.data?.data?.let { uri ->
                // Handle the selected icon URI
                Log.d("MindNote", "Selected custom icon URI: $uri")
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        workspaceName = arguments?.getString("WORKSPACE_NAME") ?: "Рабочее пространство"
        viewModel = ViewModelProvider(requireActivity())[MainViewModel::class.java]

        val view = inflater.inflate(R.layout.fragment_workspace, container, false)
        this.container = view.findViewById(R.id.container)
        
        // Find the current workspace
        currentWorkspace = viewModel.getWorkspaceById(workspaceName) ?: viewModel.getWorkspaceByName(workspaceName)
        currentWorkspace?.let { workspace ->
            viewModel.setCurrentWorkspace(workspace)
            if (!isContentLoaded) {
                loadWorkspaceContent(workspace)
                isContentLoaded = true
            }
            // Устанавливаем заголовок
            (activity as MainActivity).supportActionBar?.title = workspace.name
        }

        // Observe current workspace changes
        viewModel.currentWorkspace.observe(viewLifecycleOwner) { workspace ->
            if (workspace.id == workspaceName || workspace.name == workspaceName) {
                currentWorkspace = workspace
                if (!isContentLoaded) {
                    loadWorkspaceContent(workspace)
                    isContentLoaded = true
                }
                // Обновляем заголовок при изменении рабочего пространства
                (activity as MainActivity).supportActionBar?.title = workspace.name
                Log.d("MindNote", "WorkspaceFragment: Observed workspace '${workspace.name}' with ${workspace.items.size} items")
            }
        }

        viewModel.workspaces.observe(viewLifecycleOwner) { workspaces ->
            val updatedWorkspace = workspaces.find { it.id == workspaceName || it.name == workspaceName }
            if (updatedWorkspace != null && (currentWorkspace == null || updatedWorkspace.items.size != currentWorkspace?.items?.size)) {
                currentWorkspace = updatedWorkspace
                if (!isContentLoaded) {
                    loadWorkspaceContent(updatedWorkspace)
                    isContentLoaded = true
                }
                Log.d("MindNote", "WorkspaceFragment: Observed workspaces update, '${updatedWorkspace.name}' now has ${updatedWorkspace.items.size} items")
            }
        }

        return view
    }

    override fun onResume() {
        super.onResume()
        Log.d("MindNote", "WorkspaceFragment: onResume for workspace '${currentWorkspace?.name}'")
        // Не перезагружаем контент при возвращении из просмотра файла
        if (!isContentLoaded) {
            currentWorkspace?.let { workspace ->
                val updatedWorkspace = viewModel.getWorkspaceById(workspace.id) ?: viewModel.getWorkspaceByName(workspace.name)
                if (updatedWorkspace != null) {
                    currentWorkspace = updatedWorkspace
                    loadWorkspaceContent(updatedWorkspace)
                    isContentLoaded = true
                    Log.d("MindNote", "WorkspaceFragment: Reloaded workspace '${updatedWorkspace.name}' with ${updatedWorkspace.items.size} items")
                }
            }
        }
    }

    private fun loadWorkspaceContent(workspace: Workspace) {
        Log.d("MindNote", "WorkspaceFragment: Loading content for '${workspace.name}', ${workspace.items.size} items")
        
        // Очищаем контейнер перед загрузкой
        container.removeAllViews()
        
        // Загружаем все элементы
        workspace.items.forEach { item ->
            when (item) {
                is ContentItem.TextItem -> addTextField(item)
                is ContentItem.CheckboxItem -> addCheckboxItem(item)
                is ContentItem.FileItem -> addFileItem(item)
                is ContentItem.ImageItem -> addImageView(item)
                is ContentItem.NestedPageItem -> addNestedPageItem(item)
            }
        }
    }

    private suspend fun loadImage(uriString: String): Bitmap? = withContext(Dispatchers.IO) {
        try {
            // Проверяем кэш
            imageCache[uriString]?.let { return@withContext it }

            val uri = Uri.parse(uriString)
            context?.contentResolver?.openInputStream(uri)?.use { inputStream ->
                // Сначала получаем размеры изображения
                val options = BitmapFactory.Options().apply {
                    inJustDecodeBounds = true
                }
                BitmapFactory.decodeStream(inputStream, null, options)

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

                context?.contentResolver?.openInputStream(uri)?.use { inputStream2 ->
                    val bitmap = BitmapFactory.decodeStream(inputStream2, null, loadOptions)
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
        val textItemView = layoutInflater.inflate(R.layout.formatted_text_item, null)
        val editText = textItemView.findViewById<EditText>(R.id.formattedText)
        val formattingToolbar = textItemView.findViewById<LinearLayout>(R.id.formattingToolbar)
        
        // Устанавливаем текст с поддержкой HTML
        item?.let {
            editText.setText(Html.fromHtml(it.text, Html.FROM_HTML_MODE_COMPACT))
        }

        // Показываем панель форматирования при выделении текста
        editText.addTextChangedListener(object : android.text.TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
            override fun afterTextChanged(s: android.text.Editable?) {
                // Проверяем, есть ли выделение текста
                if (editText.selectionStart != editText.selectionEnd) {
                    formattingToolbar.visibility = View.VISIBLE
                } else {
                    formattingToolbar.visibility = View.GONE
                }
            }
        })

        // Обработчики кнопок форматирования
        textItemView.findViewById<ImageButton>(R.id.btnBold).setOnClickListener {
            applyFormat(editText, "<b>", "</b>")
        }

        textItemView.findViewById<ImageButton>(R.id.btnItalic).setOnClickListener {
            applyFormat(editText, "<i>", "</i>")
        }

        textItemView.findViewById<ImageButton>(R.id.btnBulletList).setOnClickListener {
            applyListFormat(editText, "<ul><li>", "</li></ul>")
            Log.d("text", "unordered list")
        }

        textItemView.findViewById<ImageButton>(R.id.btnNumberedList).setOnClickListener {
            applyListFormat(editText, "<ol><li>", "</li></ol>")
            Log.d("text", "ordered list")
        }

        textItemView.findViewById<ImageButton>(R.id.btnHeading).setOnClickListener {
            showHeadingDialog(editText)
        }

        // Скрываем панель форматирования при потере фокуса
        editText.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                formattingToolbar.visibility = View.GONE
                saveFormattedText(editText, item)
            }
        }

        // Добавляем обработчик выделения текста
        editText.setOnTouchListener { _, event ->
            if (event.action == android.view.MotionEvent.ACTION_UP) {
                // Небольшая задержка, чтобы дать системе время обновить выделение
                editText.postDelayed({
                    if (editText.selectionStart != editText.selectionEnd) {
                        formattingToolbar.visibility = View.VISIBLE
                    } else {
                        formattingToolbar.visibility = View.GONE
                    }
                }, 100)
            }
            false
        }

        // Create swipe container
        val swipeContainer = SwipeContainer(requireContext())
        swipeContainer.addItemView(textItemView)
        swipeContainer.setOnDeleteListener {
            currentWorkspace?.let { workspace ->
                viewModel.removeContentItem(workspace, item?.id ?: "")
                container.removeView(swipeContainer)
            }
        }

        container.addView(swipeContainer)
    }

    private fun applyFormat(editText: EditText, startTag: String, endTag: String) {
        val start = editText.selectionStart
        val end = editText.selectionEnd
        if (start < end) {
            val selectedText = editText.text.subSequence(start, end).toString()
            val lines = selectedText.split("\n")
            val formattedLines = lines.map { line ->
                if (line.isNotBlank()) {
                    val spannedLine = Html.fromHtml(line, Html.FROM_HTML_MODE_COMPACT)
                    val lineHtml = Html.toHtml(spannedLine, Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE)

                    // Проверяем, содержит ли строка только указанный тег
                    val hasOnlyCurrentTag = when (startTag) {
                        "<b>" -> lineHtml == "<b>$line</b>"
                        "<i>" -> lineHtml == "<i>$line</i>"
                        else -> false
                    }

                    if (hasOnlyCurrentTag) {
                        // Убираем тег, если он полностью оборачивает строку
                        spannedLine.toString()
                    } else {
                        // Добавляем тег, не трогая другие
                        val wrapped = StringBuilder()
                            .append(startTag)
                            .append(line)
                            .append(endTag)
                            .toString()
                        val combined = SpannableStringBuilder(Html.fromHtml(wrapped, Html.FROM_HTML_MODE_COMPACT))
                        combined
                    }
                } else {
                    line
                }
            }

            val spannable = SpannableStringBuilder()
            formattedLines.forEachIndexed { index, line ->
                spannable.append(line)
                if (index != formattedLines.lastIndex) {
                    spannable.append("\n")
                }
            }

            editText.text.replace(start, end, spannable)
        }
    }

    private fun applyListFormat(editText: EditText, startTag: String, endTag: String) {
        val start = editText.selectionStart
        val end = editText.selectionEnd
        if (start < end) {
            var selectedText = editText.text.subSequence(start, end).toString()
            val lines = selectedText.split("\n")
            val isNumberedList = lines.all { it.matches(Regex("""^\d+\.\s.*$""")) }
            val isBulletList = lines.all { it.matches(Regex("""^•\s.*$""")) }

            // Если стиль совпадает — убрать маркеры
            if ((startTag.contains("ul") && isBulletList) || (startTag.contains("ol") && isNumberedList)) {
                val unformattedLines = lines.map { line ->
                    when {
                        isNumberedList -> line.replace(Regex("""^\d+\.\s"""), "")
                        isBulletList -> line.replace(Regex("""^•\s"""), "")
                        else -> line
                    }
                }
                val unformattedText = unformattedLines.joinToString("\n")
                editText.text.replace(start, end, unformattedText)
                return
            }

            // Иначе очистить старый стиль
            val cleanedLines = lines.map { line ->
                when {
                    isNumberedList -> line.replace(Regex("""^\d+\.\s"""), "")
                    isBulletList -> line.replace(Regex("""^•\s"""), "")
                    else -> line
                }
            }

            // Получить последнее число в нумерованном списке до текущей позиции
            val lastNumber = if (startTag.contains("ol")) {
                val textBeforeSelection = editText.text.subSequence(0, start).toString()
                val linesBefore = textBeforeSelection.split("\n").reversed()

                var lastNum = 0
                var foundList = false

                for (line in linesBefore) {
                    if (line.isBlank()) break
                    val matchResult = Regex("""^(\d+)\.\s.*$""").find(line)
                    if (matchResult != null) {
                        val num = matchResult.groupValues[1].toInt()
                        if (!foundList) {
                            lastNum = num
                            foundList = true
                        } else if (num == lastNum - 1) {
                            lastNum = num
                        } else {
                            break
                        }
                    } else if (foundList) {
                        break
                    }
                }

                lastNum
            } else 0

            // Формируем новый список
            val formattedLines = if (startTag.contains("ol")) {
                cleanedLines.mapIndexed { index, line ->
                    if (line.isNotEmpty()) "${lastNumber + index + 1}. $line" else line
                }
            } else {
                cleanedLines.map { line ->
                    if (line.isNotEmpty()) "• $line" else line
                }
            }

            val formattedText = formattedLines.joinToString("\n")
            editText.text.replace(start, end, formattedText)
        }
    }

    private fun showHeadingDialog(editText: EditText) {
        val start = editText.selectionStart
        val end = editText.selectionEnd
        if (start < end) {
            val selectedText = editText.text.subSequence(start, end).toString()
            val htmlText = Html.toHtml(editText.text, Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE)
            
            // Проверяем, есть ли уже форматирование заголовка
            val isHeading = htmlText.contains("<h1>") || htmlText.contains("<h2>") || htmlText.contains("<h3>")
            
            if (isHeading) {
                // Если форматирование уже есть, убираем его
                editText.text.replace(start, end, selectedText)
                return
            }
            
            // Если форматирования нет, показываем диалог выбора уровня
            val levels = arrayOf("Заголовок 1", "Заголовок 2", "Заголовок 3")
            AlertDialog.Builder(requireContext())
                .setTitle("Выберите уровень заголовка")
                .setItems(levels) { _, which ->
                    val level = which + 1
                    val formattedText = when (level) {
                        1 -> "<h1 style='font-size: 24sp;'>$selectedText</h1>"
                        2 -> "<h2 style='font-size: 20sp;'>$selectedText</h2>"
                        else -> "<h3 style='font-size: 18sp;'>$selectedText</h3>"
                    }
                    editText.text.replace(start, end, Html.fromHtml(formattedText, Html.FROM_HTML_MODE_COMPACT))
                }
                .show()
        }
    }

    private fun saveFormattedText(editText: EditText, item: ContentItem.TextItem?) {
        val htmlText = Html.toHtml(editText.text, Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE)
        if (item != null) {
            item.text = htmlText
            currentWorkspace?.let { 
                viewModel.updateContentItem(it, item)
            }
        } else {
            val newItem = ContentItem.TextItem(htmlText)
            currentWorkspace?.let { 
                viewModel.addContentItem(it, newItem)
            }
        }
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
                    Log.d("MindNote", "Updated checkbox state in workspace '${workspace.name}': id=${updatedItem.id}")
                }
            } ?: run {
                // Если это новый элемент, создаем его
                val text = editText.text.toString()
                val newItem = ContentItem.CheckboxItem(text, isChecked)
                currentWorkspace?.let { workspace ->
                    viewModel.addContentItem(workspace, newItem)
                    currentItem = newItem // Обновляем ссылку на текущий элемент
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
                                    Log.d("MindNote", "Updated checkbox text in workspace '${workspace.name}': id=${updatedItem.id}, isChecked=${checkbox.isChecked}")
                                }
                            } ?: run {
                                val newItem = ContentItem.CheckboxItem(text, checkbox.isChecked)
                                currentWorkspace?.let { workspace -> 
                                    viewModel.addContentItem(workspace, newItem)
                                    currentItem = newItem // Обновляем ссылку на текущий элемент
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

        // Create swipe container
        val swipeContainer = SwipeContainer(requireContext())
        swipeContainer.addItemView(checkboxItemView)
        swipeContainer.setOnDeleteListener {
            currentWorkspace?.let { workspace ->
                viewModel.removeContentItem(workspace, item?.id ?: "")
                container.removeView(swipeContainer)
            }
        }

        container.addView(swipeContainer)
    }

    fun addImageView(item: ContentItem.ImageItem? = null) {
        if (item == null) return

        val imageView = ImageView(context)
        imageView.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            setMargins(0, 8, 0, 8)
        }
        imageView.adjustViewBounds = true
        imageView.scaleType = ImageView.ScaleType.FIT_CENTER

        // Сначала показываем плейсхолдер
        imageView.setImageResource(android.R.drawable.ic_menu_report_image)

        // Загружаем изображение в фоновом потоке
        viewLifecycleOwner.lifecycleScope.launch {
            try {
                val bitmap = loadImage(item.imageUri.toString())
                if (bitmap != null && !isDestroyed) {
                    withContext(Dispatchers.Main) {
                        imageView.setImageBitmap(bitmap)
                    }
                } else {
                    Log.e("MindNote", "Failed to load image: ${item.imageUri}")
                    withContext(Dispatchers.Main) {
                        imageView.setImageResource(android.R.drawable.ic_menu_report_image)
                    }
                }
            } catch (e: Exception) {
                Log.e("MindNote", "Failed to load image: ${item.imageUri}", e)
                withContext(Dispatchers.Main) {
                    imageView.setImageResource(android.R.drawable.ic_menu_report_image)
                }
            }
        }

        // Create swipe container
        val swipeContainer = SwipeContainer(requireContext())
        swipeContainer.addItemView(imageView)
        swipeContainer.setOnDeleteListener {
            currentWorkspace?.let { workspace ->
                viewModel.removeContentItem(workspace, item.id)
                container.removeView(swipeContainer)
            }
        }

        container.addView(swipeContainer)
    }

    fun addFileItem(item: ContentItem.FileItem? = null) {
        if (item == null) return

        try {
            val fileButtonView = layoutInflater.inflate(R.layout.file_button_item, null)
            val fileButton = fileButtonView.findViewById<Button>(R.id.fileButton)

            // Устанавливаем текст кнопки
            fileButton.text = item.fileName

            // Обработка нажатия на кнопку
            fileButton.setOnClickListener {
                try {
                    val intent = Intent(Intent.ACTION_VIEW).apply {
                        setDataAndType(item.fileUri, getMimeType(item.fileName))
                        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                    }
                    startActivity(intent)
                } catch (e: Exception) {
                    Log.e("MindNote", "Failed to open file: ${item.fileUri}", e)
                    Toast.makeText(context, "Не удалось открыть файл", Toast.LENGTH_SHORT).show()
                }
            }

            // Create swipe container
            val swipeContainer = SwipeContainer(requireContext())
            swipeContainer.addItemView(fileButtonView)
            swipeContainer.setOnDeleteListener {
                currentWorkspace?.let { workspace ->
                    viewModel.removeContentItem(workspace, item.id)
                    container.removeView(swipeContainer)
                }
            }

            container.addView(swipeContainer)
        } catch (e: Exception) {
            Log.e("MindNote", "Error adding file item", e)
            Toast.makeText(context, "Ошибка при добавлении файла: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }

    private fun addNestedPageItem(item: ContentItem.NestedPageItem? = null) {
        if (item == null) return

        val nestedPageView = layoutInflater.inflate(R.layout.nested_page_item, null)
        val pageNameView = nestedPageView.findViewById<TextView>(R.id.pageName)
        val pageIconView = nestedPageView.findViewById<ImageView>(R.id.pageIcon)
        
        pageNameView.text = item.pageName
        
        // Устанавливаем иконку из рабочего пространства
        val workspace = viewModel.getWorkspaceById(item.pageId)
        workspace?.iconUri?.let { uri ->
            try {
                val inputStream = requireContext().contentResolver.openInputStream(uri)
                val bitmap = BitmapFactory.decodeStream(inputStream)
                pageIconView.setImageBitmap(bitmap)
            } catch (e: Exception) {
                Log.e("MindNote", "Error loading icon: ${uri}", e)
                // Если не удалось загрузить иконку, используем стандартную
                pageIconView.setImageResource(android.R.drawable.ic_menu_agenda)
            }
        } ?: run {
            // Если иконка не установлена, используем стандартную
            pageIconView.setImageResource(android.R.drawable.ic_menu_agenda)
        }

        nestedPageView.setOnClickListener {
            // Navigate to the nested page
            val fragment = WorkspaceFragment.newInstance(item.pageId)
            parentFragmentManager.beginTransaction()
                .replace(R.id.fragment_container, fragment)
                .addToBackStack(null)
                .commit()
        }

        // Create swipe container
        val swipeContainer = SwipeContainer(requireContext())
        swipeContainer.addItemView(nestedPageView)
        swipeContainer.setOnDeleteListener {
            currentWorkspace?.let { workspace ->
                viewModel.removeContentItem(workspace, item.id)
                container.removeView(swipeContainer)
            }
        }

        container.addView(swipeContainer)
    }

    fun showCreateNestedPageDialog() {
        val dialogView = layoutInflater.inflate(R.layout.dialog_add_menu_item, null)
        val editText = dialogView.findViewById<EditText>(R.id.editTextMenuItem)
        val buttonSelectIcon = dialogView.findViewById<Button>(R.id.buttonSelectIcon)
        editText.hint = "Название страницы"

        buttonSelectIcon.setOnClickListener {
            showIconSelectionDialog { selectedIconUri ->
                // Здесь можно сохранить выбранную иконку, если нужно
                Log.d("MindNote", "Selected icon URI: $selectedIconUri")
            }
        }

        AlertDialog.Builder(requireContext())
            .setTitle("Создать вложенную страницу")
            .setView(dialogView)
            .setPositiveButton("Создать") { _, _ ->
                val pageName = editText.text.toString()
                if (pageName.isNotEmpty()) {
                    // Создаем новое рабочее пространство для вложенной страницы
                    val nestedWorkspace = viewModel.createWorkspace(pageName)
                    // Создаем элемент вложенной страницы
                    val nestedPageItem = ContentItem.NestedPageItem(
                        pageName = pageName,
                        pageId = nestedWorkspace.id
                    )
                    // Добавляем элемент в текущее рабочее пространство
                    currentWorkspace?.let { workspace ->
                        viewModel.addContentItem(workspace, nestedPageItem)
                        addNestedPageItem(nestedPageItem)
                    }
                }
            }
            .setNegativeButton("Отмена", null)
            .show()
    }

    private fun showIconSelectionDialog(onIconSelected: (Uri?) -> Unit) {
        val dialogView = layoutInflater.inflate(R.layout.dialog_select_icon, null)
        val dialog = AlertDialog.Builder(requireContext())
            .setTitle("Выберите иконку")
            .setView(dialogView)
            .setNegativeButton("Отмена", null)
            .create()

        // Обработчики для стандартных иконок
        val iconButtons = listOf(
            dialogView.findViewById<ImageButton>(R.id.icon1),
            dialogView.findViewById<ImageButton>(R.id.icon2),
            dialogView.findViewById<ImageButton>(R.id.icon3),
            dialogView.findViewById<ImageButton>(R.id.icon4),
            dialogView.findViewById<ImageButton>(R.id.icon5),
            dialogView.findViewById<ImageButton>(R.id.icon6),
            dialogView.findViewById<ImageButton>(R.id.icon7),
            dialogView.findViewById<ImageButton>(R.id.icon8)
        )

        iconButtons.forEachIndexed { index, button ->
            button.setOnClickListener {
                val iconResId = resources.obtainTypedArray(R.array.workspace_icons).getResourceId(index, 0)
                val iconUri = Uri.parse("android.resource://${requireContext().packageName}/$iconResId")
                onIconSelected(iconUri)
                dialog.dismiss()
            }
        }

        // Обработчик для кнопки выбора своей иконки
        dialogView.findViewById<Button>(R.id.buttonCustomIcon).setOnClickListener {
            val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                addCategory(Intent.CATEGORY_OPENABLE)
                type = "image/*"
            }
            pickIconLauncher.launch(intent)
            dialog.dismiss()
        }

        dialog.show()
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
            fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") -> "image/jpeg"
            fileName.endsWith(".png") -> "image/png"
            fileName.endsWith(".gif") -> "image/gif"
            else -> "application/octet-stream"
        }
    }

    override fun onPause() {
        super.onPause()
        Log.d("MindNote", "WorkspaceFragment: onPause for workspace '${currentWorkspace?.name}'")
    }

    override fun onStop() {
        super.onStop()
        Log.d("MindNote", "WorkspaceFragment: onStop for workspace '${currentWorkspace?.name}'")
    }

    override fun onDestroy() {
        super.onDestroy()
        isDestroyed = true
        isContentLoaded = false
        // Очищаем кэш изображений
        imageCache.clear()
        Log.d("MindNote", "WorkspaceFragment: onDestroy for workspace '${currentWorkspace?.name}'")
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
    }

    companion object {
        private const val PICK_ICON_REQUEST_CODE = 1001

        fun newInstance(workspaceName: String): WorkspaceFragment {
            val fragment = WorkspaceFragment()
            val args = Bundle()
            args.putString("WORKSPACE_NAME", workspaceName)
            fragment.arguments = args
            return fragment
        }
    }

    private fun showLoadingDialog(): AlertDialog {
        val dialogView = layoutInflater.inflate(R.layout.dialog_loading, null)
        val dialog = AlertDialog.Builder(requireContext())
            .setView(dialogView)
            .setCancelable(false)
            .create()
        dialog.show()
        return dialog
    }
}
