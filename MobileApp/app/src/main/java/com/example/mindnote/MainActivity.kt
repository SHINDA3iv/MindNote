package com.example.mindnote

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.lifecycle.ViewModelProvider
import com.example.mindnote.viewmodel.MindNoteViewModel
import com.google.android.material.navigation.NavigationView

class MainActivity : AppCompatActivity() {
    private lateinit var drawerLayout: DrawerLayout
    private lateinit var navigationView: NavigationView
    private lateinit var viewModel: MindNoteViewModel
    private val PICK_IMAGE_REQUEST = 1
    private var imageUri: Uri? = null
    private lateinit var toolbarAddButton: MenuItem

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Initialize ViewModel
        viewModel = ViewModelProvider(this)[MindNoteViewModel::class.java]

        drawerLayout = findViewById(R.id.drawer_layout)
        navigationView = findViewById(R.id.nav_view)

        val toolbar = findViewById<Toolbar>(R.id.toolbar)
        setSupportActionBar(toolbar)

        // Add button for opening side menu
        ActionBarDrawerToggle(this, drawerLayout, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close).also { toggle ->
            drawerLayout.addDrawerListener(toggle)
            toggle.syncState()
        }

        // Setup navigation header add button
        val headerView = navigationView.getHeaderView(0)
        val addButton = headerView.findViewById<Button>(R.id.nav_header_add_button)
        addButton.setOnClickListener {
            showAddWorkspaceDialog()
        }

        // Handle navigation item clicks
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

        // Observe workspaces and update navigation menu
        viewModel.allWorkspaces.observe(this) { workspaces ->
            navigationView.menu.clear()
            workspaces.forEach { workspace ->
                navigationView.menu.add(R.id.ws_group, workspace.id.toInt(), Menu.NONE, workspace.name)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        toolbarAddButton = menu?.findItem(R.id.action_add) ?: return false
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.popup_option1 -> {
                val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                fragment?.addTextField()
                true
            }
            R.id.popup_option2 -> {
                val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
                fragment?.addCheckboxItem("")
                true
            }
            R.id.popup_option3 -> {
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
            imageUri = data.data
            val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container) as? WorkspaceFragment
            fragment?.addImageView(imageUri)
        }
    }

    private fun showAddWorkspaceDialog() {
        val builder = AlertDialog.Builder(this)
        val dialogView = layoutInflater.inflate(R.layout.dialog_add_menu_item, null)
        val editText = dialogView.findViewById<EditText>(R.id.editTextMenuItem)

        builder.setTitle("Создание нового пространства")
            .setView(dialogView)
            .setPositiveButton("Создать") { _, _ ->
                val workspaceName = editText.text.toString().trim()
                if (workspaceName.isNotEmpty()) {
                    viewModel.createWorkspace(workspaceName)
                } else {
                    Toast.makeText(this, "Имя пространства не может быть пустым", Toast.LENGTH_SHORT).show()
                }
            }
            .setNegativeButton("Отмена") { dialog, _ -> dialog.cancel() }

        builder.show()
    }

    private fun openWorkspace(workspaceName: String) {
        val fragment = WorkspaceFragment.newInstance(workspaceName)
        supportFragmentManager.beginTransaction()
            .replace(R.id.fragment_container, fragment)
            .addToBackStack(null)
            .commit()
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