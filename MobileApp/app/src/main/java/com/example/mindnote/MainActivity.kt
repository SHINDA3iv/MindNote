package com.example.mindnote

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.Button
import android.widget.CheckBox
import android.widget.EditText
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.PopupMenu
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.widget.Toolbar
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.GravityCompat
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.lifecycle.ViewModelProvider
import com.example.mindnote.R.id.nav_header_add_button
import com.example.mindnote.R.id.ws_group
import com.example.mindnote.data.ContentItem
import com.example.mindnote.data.Workspace
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.navigation.NavigationView
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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        drawerLayout = findViewById(R.id.drawer_layout)
        navigationView = findViewById(R.id.nav_view)
        viewModel = ViewModelProvider(this)[MainViewModel::class.java]

        val toolbar = findViewById<Toolbar>(R.id.toolbar)
        setSupportActionBar(toolbar)

        // Добавление кнопки для открытия бокового меню
        ActionBarDrawerToggle(this, drawerLayout, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close).also { toggle ->
            drawerLayout.addDrawerListener(toggle)
            toggle.syncState()
        }

        val headerView = navigationView.getHeaderView(0)
        val addButton = headerView.findViewById<Button>(R.id.nav_header_add_button)
        addButton.setOnClickListener {
            showAddMenuItemDialog()
        }

        // Обработка нажатий на элементы меню
        navigationView.setNavigationItemSelectedListener { menuItem ->
            // Получаем имя рабочего пространства из заголовка пункта меню
            val workspaceName = menuItem.title.toString()

            // Обрабатываем нажатие на любой элемент
            if (menuItem.itemId != -1) { // Проверяем, что itemId валиден
                openWorkspace(workspaceName) // Открываем рабочее пространство
                drawerLayout.closeDrawer(GravityCompat.START) // Закрываем навигационное меню
                true
            } else {
                false
            }
        }

        // Load saved workspaces
        viewModel.loadWorkspaces(this)
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        toolbarAddButton = menu?.findItem(R.id.action_add) ?: return false

        toolbarAddButton.setOnMenuItemClickListener {
            showPopupMenu()
            true
        }

        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.popup_option1 -> {
                // Добавляем поле для ввода текста
                val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                fragment.addTextField()
                true
            }
            R.id.popup_option2 -> {
                // Добавляем чекбокс
                val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                val checkboxItem = ContentItem.CheckboxItem(
                    text = "",
                    isChecked = false,
                    id = java.util.UUID.randomUUID().toString()
                )
                fragment.addCheckboxItem(checkboxItem)
                true
            }
            R.id.popup_option3 -> {
                // Открываем галерею для выбора изображения
                val intent = Intent(Intent.ACTION_PICK)
                intent.type = "image/*"
                startActivityForResult(intent, PICK_IMAGE_REQUEST)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == Activity.RESULT_OK && data != null) {
            when (requestCode) {
                PICK_IMAGE_REQUEST -> {
                    imageUri = data.data
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                    fragment.addImageView(ContentItem.ImageItem(imageUri!!))
                }
                PICK_FILE_REQUEST -> {
                    data.data?.let { uri ->
                        val fileName = getFileName(uri)
                        val fileSize = getFileSize(uri)
                        val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                        fragment.addFileItem(ContentItem.FileItem(fileName, uri, fileSize))
                    }
                }
                PICK_ICON_REQUEST -> {
                    data.data?.let { uri ->
                        val workspaceName = data.getStringExtra("workspace_name")
                        workspaceName?.let { name ->
                            val workspace = viewModel.workspaces.value?.find { it.name == name }
                            workspace?.let {
                                it.iconUri = uri
                                viewModel.updateWorkspace(it)
                                updateWorkspaceMenuItem(name, uri)
                            }
                        }
                    }
                }
            }
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

    private fun showPopupMenu() {
        val popupMenu = PopupMenu(this, findViewById(R.id.action_add))
        popupMenu.menuInflater.inflate(R.menu.popup_menu, popupMenu.menu)
        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.popup_option1 -> {
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                    fragment.addTextField()
                    true
                }
                R.id.popup_option2 -> {
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                    val checkboxItem = ContentItem.CheckboxItem(
                        text = "",
                        isChecked = false,
                        id = java.util.UUID.randomUUID().toString()
                    )
                    fragment.addCheckboxItem(checkboxItem)
                    true
                }
                R.id.popup_option3 -> {
                    val intent = Intent(Intent.ACTION_PICK)
                    intent.type = "image/*"
                    startActivityForResult(intent, PICK_IMAGE_REQUEST)
                    true
                }
                R.id.popup_option4 -> {
                    val intent = Intent(Intent.ACTION_GET_CONTENT)
                    intent.type = "*/*"
                    intent.addCategory(Intent.CATEGORY_OPENABLE)
                    startActivityForResult(intent, PICK_FILE_REQUEST)
                    true
                }
                R.id.popup_option5 -> {
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                    fragment.addNumberedListItem(null)
                    true
                }
                R.id.popup_option6 -> {
                    val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
                    fragment.addBulletListItem(null)
                    true
                }
                else -> false
            }
        }
        popupMenu.show()
    }

    private fun openWorkspace(workspaceName: String) {
        val fragment = WorkspaceFragment.newInstance(workspaceName)
        supportFragmentManager.beginTransaction()
            .replace(R.id.fragment_container, fragment)
            .addToBackStack(null)
            .commit()
    }

    private fun showAddMenuItemDialog() {
        val builder = AlertDialog.Builder(this)
        val dialogView = layoutInflater.inflate(R.layout.dialog_add_menu_item, null)
        val editText = dialogView.findViewById<EditText>(R.id.editTextMenuItem)
        val iconButton = dialogView.findViewById<Button>(R.id.buttonSelectIcon)

        var selectedIconUri: Uri? = null

        iconButton.setOnClickListener {
            val intent = Intent(Intent.ACTION_PICK)
            intent.type = "image/*"
            intent.putExtra("workspace_name", editText.text.toString())
            startActivityForResult(intent, PICK_ICON_REQUEST)
        }

        builder.setTitle("Создание нового пространства")
            .setView(dialogView)
            .setPositiveButton("Создать пространство") { dialog, which ->
                val newItemName = editText.text.toString().trim()
                if (newItemName.isNotEmpty()) {
                    val workspace = viewModel.createWorkspace(newItemName, selectedIconUri)
                    addMenuItem(newItemName, workspace)
                    editText.text.clear()
                } else {
                    Toast.makeText(this, "Имя пункта не может быть пустым", Toast.LENGTH_SHORT).show()
                }
            }
            .setNegativeButton("Отмена") { dialog, which -> dialog.cancel() }

        builder.show()
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

    override fun onPause() {
        super.onPause()
        viewModel.saveWorkspaces(this)
    }
}