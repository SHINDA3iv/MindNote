package com.example.mindnote

import android.app.Activity
import android.app.ProgressDialog
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.*
import androidx.appcompat.widget.Toolbar
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import com.example.mindnote.R.id.nav_header_add_button
import com.example.mindnote.R.id.ws_group
import com.example.mindnote.data.ContentItem
import com.example.mindnote.data.Workspace
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.navigation.NavigationView
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File

class MainActivity : AppCompatActivity() {
    private lateinit var drawerLayout: DrawerLayout
    private lateinit var navigationView: NavigationView
    private lateinit var viewModel: MainViewModel
    private val PICK_IMAGE_REQUEST = 1
    private val PICK_FILE_REQUEST = 2
    private val PICK_ICON_REQUEST = 3
    private var imageUri: Uri? = null
    private lateinit var toolbarAddButton: MenuItem
    private var selectedIconUri: Uri? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        enableEdgeToEdge()
        drawerLayout = findViewById(R.id.drawer_layout)
        navigationView = findViewById(R.id.nav_view)
        viewModel = ViewModelProvider(this)[MainViewModel::class.java]
        
        // Инициализируем ViewModel
        viewModel.init(applicationContext)
        
        val toolbar = findViewById<Toolbar>(R.id.toolbar)
        setSupportActionBar(toolbar)

        // Навигация
        val toggle = ActionBarDrawerToggle(this, drawerLayout, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close)
        drawerLayout.addDrawerListener(toggle)
        toggle.syncState()

        val headerView = navigationView.getHeaderView(0)
        val addButton = headerView.findViewById<Button>(R.id.nav_header_add_button)
        addButton.setOnClickListener {
            showAddMenuItemDialog()
        }

        navigationView.setNavigationItemSelectedListener { menuItem ->
            val workspaceName = menuItem.title.toString()
            if (menuItem.itemId != -1) {
                openWorkspace(workspaceName)
                drawerLayout.closeDrawer(GravityCompat.START)
                true
            } else {
                false
            }
        }

        // Восстановление фрагмента если есть
        if (savedInstanceState != null) {
            val currentFragment = supportFragmentManager.findFragmentById(R.id.fragment_container)
            if (currentFragment is WorkspaceFragment) {
                Log.d("MindNote", "Fragment restored after configuration change")
            } else {
                // Если фрагмент не найден и есть текущее рабочее пространство, откроем его
                viewModel.currentWorkspace.value?.let { workspace ->
                    Log.d("MindNote", "Reopening current workspace after configuration change")
                    openWorkspace(workspace.name)
                }
            }
        }

        // Подписываемся на обновления
        viewModel.workspaces.observe(this) { workspaces ->
            Log.d("MindNote", "MainActivity: Observed ${workspaces.size} workspaces")
            val menu = navigationView.menu
            menu.removeGroup(ws_group)
            workspaces.forEach { workspace ->
                addMenuItem(workspace.name, workspace)
                Log.d("MindNote", "MainActivity: Added menu item for '${workspace.name}' with ${workspace.items.size} items")
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)

        // Инициализируем пункт меню
        toolbarAddButton = menu?.findItem(R.id.action_add) ?: return false
        toolbarAddButton.setOnMenuItemClickListener {
            showPopupMenu()
            true
        }

        // По умолчанию скрываем кнопку
        toolbarAddButton.isVisible = false

        return true
    }

    private fun showPopupMenu() {
        val popupMenu = PopupMenu(this, findViewById(R.id.action_add))
        popupMenu.menuInflater.inflate(R.menu.popup_menu, popupMenu.menu)
        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.popup_option1 -> {
                    Log.d("MindNote", "MainActivity: Add text field selected")
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                    fragment?.addTextField()
                    true
                }
                R.id.popup_option2 -> {
                    Log.d("MindNote", "MainActivity: Add checkbox selected")
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                    fragment?.let {
                        val checkboxItem = ContentItem.CheckboxItem(
                            text = "",
                            isChecked = false,
                            id = java.util.UUID.randomUUID().toString()
                        )
                        it.addCheckboxItem(checkboxItem)
                    }
                    true
                }
                R.id.popup_option3 -> {
                    Log.d("MindNote", "MainActivity: Add image selected")
                    val intent = Intent(Intent.ACTION_PICK)
                    intent.type = "image/*"
                    startActivityForResult(intent, PICK_IMAGE_REQUEST)
                    true
                }
                R.id.popup_option4 -> {
                    Log.d("MindNote", "MainActivity: Add file selected")
                    val intent = Intent(Intent.ACTION_GET_CONTENT)
                    intent.type = "*/*"
                    intent.addCategory(Intent.CATEGORY_OPENABLE)
                    startActivityForResult(intent, PICK_FILE_REQUEST)
                    true
                }
                R.id.popup_option5 -> {
                    Log.d("MindNote", "MainActivity: Add nested page selected")
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                    fragment?.showCreateNestedPageDialog()
                    true
                }
                else -> false
            }
        }
        popupMenu.show()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == Activity.RESULT_OK && data != null) {
            when (requestCode) {
                PICK_IMAGE_REQUEST -> {
                    data.data?.let { uri ->
                        Log.d("MindNote", "MainActivity: Image picked: $uri")
                        try {
                            // Проверяем размер файла
                            val fileSize = getFileSize(uri)
                            if (fileSize > 10 * 1024 * 1024) { // 10MB limit
                                Toast.makeText(this, "Изображение слишком большое. Максимальный размер: 10MB", Toast.LENGTH_LONG).show()
                                return
                            }
                            
                            // Показываем прогресс
                            val progressDialog = ProgressDialog(this).apply {
                                setMessage("Обработка изображения...")
                                setCancelable(false)
                                show()
                            }
                            
                            // Копируем изображение в фоновом потоке
                            lifecycleScope.launch(Dispatchers.IO) {
                                try {
                                    // Проверяем доступность файла
                                    contentResolver.openInputStream(uri)?.use { input ->
                                        // Проверяем, что это действительно изображение
                                        val options = BitmapFactory.Options().apply {
                                            inJustDecodeBounds = true
                                        }
                                        BitmapFactory.decodeStream(input, null, options)
                                        if (options.outWidth <= 0 || options.outHeight <= 0) {
                                            throw Exception("Invalid image format")
                                        }
                                    } ?: throw Exception("Cannot open image file")

                                    val persistentUri = copyFileToInternalStorage(uri, "images")
                                    Log.d("MindNote", "Image copied to: $persistentUri")
                                    
                                    withContext(Dispatchers.Main) {
                                        progressDialog.dismiss()
                                        val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                                        if (fragment != null) {
                                            val imageItem = ContentItem.ImageItem(persistentUri)
                                            fragment.addImageView(imageItem)
                                            Log.d("MindNote", "Image added to workspace")
                                        } else {
                                            Log.e("MindNote", "Fragment not found")
                                            Toast.makeText(this@MainActivity, "Ошибка: фрагмент не найден", Toast.LENGTH_LONG).show()
                                        }
                                    }
                                } catch (e: Exception) {
                                    Log.e("MindNote", "Error processing image", e)
                                    withContext(Dispatchers.Main) {
                                        progressDialog.dismiss()
                                        Toast.makeText(this@MainActivity, 
                                            "Ошибка при обработке изображения: ${e.message}", 
                                            Toast.LENGTH_LONG).show()
                                    }
                                }
                            }
                        } catch (e: Exception) {
                            Log.e("MindNote", "Error handling image pick", e)
                            Toast.makeText(this, 
                                "Ошибка при выборе изображения: ${e.message}", 
                                Toast.LENGTH_LONG).show()
                        }
                    }
                }
                PICK_FILE_REQUEST -> {
                    data.data?.let { uri ->
                        val progressDialog = ProgressDialog(this).apply {
                            setMessage("Обработка файла...")
                            setCancelable(false)
                            show()
                        }

                        lifecycleScope.launch(Dispatchers.IO) {
                            try {
                                val fileName = getFileName(uri)
                                val fileSize = getFileSize(uri)
                                Log.d("MindNote", "MainActivity: File picked: $fileName, size: $fileSize")

                                // Проверяем доступность файла
                                contentResolver.openInputStream(uri)?.use { input ->
                                    if (input.available() <= 0) {
                                        throw Exception("File is empty or not accessible")
                                    }
                                } ?: throw Exception("Cannot open file")

                                val persistentUri = copyFileToInternalStorage(uri, "files", fileName)
                                Log.d("MindNote", "File copied to: $persistentUri")

                                withContext(Dispatchers.Main) {
                                    progressDialog.dismiss()
                                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                                    if (fragment != null) {
                                        val fileItem = ContentItem.FileItem(
                                            fileName = fileName,
                                            fileUri = persistentUri,
                                            fileSize = fileSize,
                                            id = java.util.UUID.randomUUID().toString()
                                        )
                                        fragment.addFileItem(fileItem)
                                        Log.d("MindNote", "File added to workspace: $fileName")
                                    } else {
                                        Log.e("MindNote", "Fragment not found")
                                        Toast.makeText(this@MainActivity, "Ошибка: фрагмент не найден", Toast.LENGTH_LONG).show()
                                    }
                                }
                            } catch (e: Exception) {
                                Log.e("MindNote", "Error processing file", e)
                                withContext(Dispatchers.Main) {
                                    progressDialog.dismiss()
                                    Toast.makeText(this@MainActivity, 
                                        "Ошибка при обработке файла: ${e.message}", 
                                        Toast.LENGTH_LONG).show()
                                }
                            }
                        }
                    }
                }
                PICK_ICON_REQUEST -> {
                    data.data?.let { uri ->
                        // Копируем иконку в приватное хранилище приложения
                        val persistentUri = copyFileToInternalStorage(uri, "icons", "temp_icon")
                        // Обновляем выбранную иконку
                        selectedIconUri = persistentUri
                        // Показываем уведомление об успешном выборе иконки
                        Toast.makeText(this, "Иконка выбрана", Toast.LENGTH_SHORT).show()
                    }
                }
            }
        }
    }

    private fun copyFileToInternalStorage(sourceUri: Uri, dirName: String, customFileName: String? = null): Uri {
        val inputStream = contentResolver.openInputStream(sourceUri)
            ?: throw Exception("Cannot open source file")
            
        val fileName = customFileName ?: getFileName(sourceUri)
        val timeStamp = System.currentTimeMillis()
        val safeFileName = "$timeStamp-${fileName.replace("[^a-zA-Z0-9.-]".toRegex(), "_")}"
        
        // Создаем директорию если её нет
        val directory = File(filesDir, dirName)
        if (!directory.exists()) {
            directory.mkdirs()
        }
        
        // Создаем файл
        val file = File(directory, safeFileName)
        Log.d("MindNote", "Copying file to: ${file.absolutePath}")
        
        try {
            // Копируем данные
            inputStream.use { input ->
                file.outputStream().use { output ->
                    input.copyTo(output)
                }
            }
            
            // Возвращаем URI для сохраненного файла через FileProvider
            return androidx.core.content.FileProvider.getUriForFile(
                this,
                "${packageName}.fileprovider",
                file
            )
        } catch (e: Exception) {
            Log.e("MindNote", "Error copying file", e)
            throw Exception("Failed to copy file: ${e.message}")
        }
    }

    private fun getFileName(uri: Uri): String {
        val cursor = contentResolver.query(uri, null, null, null, null)
        cursor?.use {
            if (it.moveToFirst()) {
                val nameIndex = it.getColumnIndex(android.provider.OpenableColumns.DISPLAY_NAME)
                if (nameIndex != -1) {
                    return it.getString(nameIndex)
                }
            }
        }
        return "Unknown file"
    }

    private fun getFileSize(uri: Uri): Long {
        val cursor = contentResolver.query(uri, null, null, null, null)
        cursor?.use {
            if (it.moveToFirst()) {
                val sizeIndex = it.getColumnIndex(android.provider.OpenableColumns.SIZE)
                if (sizeIndex != -1) {
                    return it.getLong(sizeIndex)
                }
            }
        }
        return 0L
    }

    private fun openWorkspace(workspaceName: String) {
        val workspace = viewModel.workspaces.value?.find { it.name == workspaceName }
        workspace?.let {
            Log.d("MindNote", "MainActivity: Opening workspace '${workspace.name}' with ${workspace.items.size} items")
            viewModel.setCurrentWorkspace(it)

            // Очищаем стек фрагментов перед добавлением нового
            supportFragmentManager.popBackStack(null, androidx.fragment.app.FragmentManager.POP_BACK_STACK_INCLUSIVE)
            val fragment = WorkspaceFragment.newInstance(workspaceName)
            supportFragmentManager.beginTransaction()
                .replace(R.id.fragment_container, fragment)
                .commit()

            // Показываем кнопку меню
            toolbarAddButton.isVisible = true

            // Обновляем заголовок и иконку в toolbar
            val toolbar = findViewById<Toolbar>(R.id.toolbar)
            toolbar.title = workspaceName
            
            // Устанавливаем иконку в toolbar
            workspace.iconUri?.let { uri ->
                try {
                    val icon = contentResolver.openInputStream(uri)?.use {
                        android.graphics.BitmapFactory.decodeStream(it)
                    }
                    icon?.let {
                        val scaledIcon = android.graphics.Bitmap.createScaledBitmap(it, 48, 48, true)
                        toolbar.logo = android.graphics.drawable.BitmapDrawable(resources, scaledIcon)
                    }
                } catch (e: Exception) {
                    Log.e("MindNote", "Error setting toolbar icon", e)
                    toolbar.logo = null
                }
            } ?: run {
                toolbar.logo = null
            }
        }
    }

    private fun showAddMenuItemDialog() {
        val builder = AlertDialog.Builder(this)
        val dialogView = layoutInflater.inflate(R.layout.dialog_add_menu_item, null)
        val editText = dialogView.findViewById<EditText>(R.id.editTextMenuItem)
        val iconButton = dialogView.findViewById<Button>(R.id.buttonSelectIcon)

        iconButton.setOnClickListener {
            showIconSelectionDialog()
        }

        builder.setTitle("Создание нового пространства")
            .setView(dialogView)
            .setPositiveButton("Создать пространство") { dialog, which ->
                val newItemName = editText.text.toString().trim()
                if (newItemName.isNotEmpty()) {
                    Log.d("MindNote", "MainActivity: Creating new workspace '$newItemName'")
                    val workspace = viewModel.createWorkspace(newItemName, selectedIconUri)
                    addMenuItem(newItemName, workspace)
                    editText.text.clear()
                    selectedIconUri = null
                } else {
                    Toast.makeText(this, "Имя пункта не может быть пустым", Toast.LENGTH_SHORT).show()
                }
            }
            .setNegativeButton("Отмена") { dialog, which -> 
                dialog.cancel()
                selectedIconUri = null
            }
        builder.show()
    }

    private fun showIconSelectionDialog() {
        val dialogView = layoutInflater.inflate(R.layout.dialog_select_icon, null)
        val dialog = AlertDialog.Builder(this)
            .setView(dialogView)
            .create()

        // Устанавливаем обработчики для каждой иконки
        val iconIds = arrayOf(
            R.id.icon1, R.id.icon2, R.id.icon3, R.id.icon4,
            R.id.icon5, R.id.icon6, R.id.icon7, R.id.icon8
        )

        iconIds.forEachIndexed { index, id ->
            dialogView.findViewById<ImageButton>(id).setOnClickListener {
                try {
                    // Получаем ID ресурса иконки
                    val typedArray = resources.obtainTypedArray(R.array.workspace_icons)
                    val iconResId = typedArray.getResourceId(index, 0)
                    typedArray.recycle()

                    if (iconResId == 0) {
                        throw Exception("Не удалось получить ресурс иконки")
                    }

                    // Создаем директорию для иконок
                    val iconsDir = File(filesDir, "icons")
                    if (!iconsDir.exists() && !iconsDir.mkdirs()) {
                        throw Exception("Не удалось создать директорию для иконок")
                    }

                    // Сохраняем иконку
                    val file = File(iconsDir, "icon_$index.png")
                    
                    // Получаем векторный drawable
                    val drawable = resources.getDrawable(iconResId, theme)
                    
                    // Создаем bitmap с поддержкой прозрачности
                    val bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888)
                    val canvas = android.graphics.Canvas(bitmap)
                    
                    // Очищаем canvas прозрачным цветом
                    canvas.drawColor(android.graphics.Color.TRANSPARENT, android.graphics.PorterDuff.Mode.CLEAR)
                    
                    // Устанавливаем границы drawable
                    drawable.setBounds(0, 0, canvas.width, canvas.height)
                    
                    // Рисуем drawable на canvas
                    drawable.draw(canvas)

                    // Сохраняем bitmap в файл
                    file.outputStream().use { out ->
                        if (!bitmap.compress(Bitmap.CompressFormat.PNG, 100, out)) {
                            throw Exception("Не удалось сохранить иконку")
                        }
                    }

                    // Освобождаем память
                    bitmap.recycle()

                    // Создаем URI через FileProvider
                    selectedIconUri = androidx.core.content.FileProvider.getUriForFile(
                        this,
                        "${packageName}.fileprovider",
                        file
                    )

                    // Показываем уведомление об успешном выборе
                    Toast.makeText(this, "Иконка выбрана", Toast.LENGTH_SHORT).show()
                    dialog.dismiss()
                } catch (e: Exception) {
                    Log.e("MindNote", "Error saving icon", e)
                    Toast.makeText(this, "Ошибка при сохранении иконки: ${e.message}", Toast.LENGTH_SHORT).show()
                }
            }
        }

        // Обработчик для кнопки выбора своей иконки
        dialogView.findViewById<Button>(R.id.buttonCustomIcon).setOnClickListener {
            dialog.dismiss()
            try {
                val intent = Intent(Intent.ACTION_PICK)
                intent.type = "image/*"
                startActivityForResult(intent, PICK_ICON_REQUEST)
            } catch (e: Exception) {
                Log.e("MindNote", "Error launching image picker", e)
                Toast.makeText(this, "Ошибка при открытии галереи: ${e.message}", Toast.LENGTH_SHORT).show()
            }
        }

        dialog.show()
    }

    private fun addMenuItem(itemName: String, workspace: Workspace) {
        val menu = navigationView.menu
        val itemId = generateUniqueMenuItemId(menu)
        val menuItem = menu.add(ws_group, itemId, Menu.NONE, itemName)
        workspace.iconUri?.let { uri ->
            val icon = contentResolver.openInputStream(uri)?.use {
                android.graphics.BitmapFactory.decodeStream(it)
            }
            icon?.let {
                val scaledIcon = android.graphics.Bitmap.createScaledBitmap(it, 48, 48, true)
                menuItem.icon = android.graphics.drawable.BitmapDrawable(resources, scaledIcon)
            }
        }
    }

    private fun updateWorkspaceMenuItem(workspaceName: String, iconUri: Uri) {
        val menu = navigationView.menu
        for (i in 0 until menu.size()) {
            val menuItem = menu.getItem(i)
            if (menuItem.title.toString() == workspaceName) {
                val icon = contentResolver.openInputStream(iconUri)?.use {
                    android.graphics.BitmapFactory.decodeStream(it)
                }
                icon?.let {
                    val scaledIcon = android.graphics.Bitmap.createScaledBitmap(it, 48, 48, true)
                    menuItem.icon = android.graphics.drawable.BitmapDrawable(resources, scaledIcon)
                }
                break
            }
        }
    }

    private fun generateUniqueMenuItemId(menu: Menu): Int {
        var newId = 1
        while (menu.findItem(newId) != null) {
            newId++
        }
        return newId
    }

    override fun onBackPressed() {
        if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
            drawerLayout.closeDrawer(GravityCompat.START)
        } else {
            if (supportFragmentManager.backStackEntryCount > 0) {
                supportFragmentManager.popBackStack()
            } else {
                super.onBackPressed()
            }
        }
    }

    override fun onResume() {
        super.onResume()
    }

    override fun onPause() {
        super.onPause()
    }

    override fun onStop() {
        super.onStop()
    }

    override fun onDestroy() {
        super.onDestroy()
    }
}