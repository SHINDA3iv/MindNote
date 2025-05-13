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
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.navigation.NavigationView

class MainActivity : AppCompatActivity() {
    private lateinit var drawerLayout: DrawerLayout
    private lateinit var navigationView: NavigationView
    private lateinit var viewModel: MainViewModel
    private val PICK_IMAGE_REQUEST = 1
    private var imageUri: Uri? = null
    private lateinit var toolbarAddButton: MenuItem

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        drawerLayout = findViewById(R.id.drawer_layout)
        navigationView = findViewById(R.id.nav_view)

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
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        toolbarAddButton = menu?.findItem(R.id.action_add)?: return false

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
                fragment.addCheckboxItem("")
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
        if (requestCode == PICK_IMAGE_REQUEST && resultCode == Activity.RESULT_OK && data != null) {
            imageUri = data.data // Получаем URI выбранного изображения
            val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as WorkspaceFragment
            fragment.addImageView(imageUri) // Передаем URI во фрагмент
        }
    }

    private fun showPopupMenu() {
        val popupMenu = PopupMenu(this, findViewById(R.id.action_add))
        popupMenu.menuInflater.inflate(R.menu.popup_menu, popupMenu.menu) // Создайте этот файл меню, если нужно
        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {

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

        builder.setTitle("Создание нового пространства")
            .setView(dialogView)
            .setPositiveButton("Создать пространство") { dialog, which ->
                val newItemName = editText.text.toString().trim() // Удаляем пробелы
                if (newItemName.isNotEmpty()) {
                    addMenuItem(newItemName)
                    editText.text.clear() // Очищаем поле ввода
                } else {
                    Toast.makeText(this, "Имя пункта не может быть пустым", Toast.LENGTH_SHORT).show()
                }
            }
            .setNegativeButton("Отмена") { dialog, which -> dialog.cancel() }

        builder.show()
    }

    private fun addMenuItem(itemName: String) {
        // Получаем ссылку на меню
        val menu = navigationView.menu

        // Генерируем уникальный ID для нового пункта
        val itemId = generateUniqueMenuItemId(menu)

        // Добавляем новый пункт в меню с уникальным id
        menu.add(ws_group, itemId, Menu.NONE, itemName) // Menu.NONE для позиции
    }

    private fun generateUniqueMenuItemId(menu: Menu): Int {
        var newId = 1 // Начинаем с 1
        while (menu.findItem(newId) != null) {
            newId++ // Ищем следующий доступный ID
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


}