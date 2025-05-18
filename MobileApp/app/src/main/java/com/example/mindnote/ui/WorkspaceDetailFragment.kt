package com.example.mindnote.ui

import android.app.Activity
import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.fragment.findNavController
import androidx.navigation.fragment.navArgs
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.mindnote.R
import com.example.mindnote.data.ContentItem
import com.example.mindnote.data.Workspace
import com.example.mindnote.data.WorkspaceRepository
import com.google.android.material.chip.Chip
import com.google.android.material.chip.ChipGroup
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.textfield.TextInputEditText
import androidx.appcompat.widget.PopupMenu

class WorkspaceDetailFragment : Fragment() {
    private lateinit var viewModel: MainViewModel
    private lateinit var workspaceRepository: WorkspaceRepository
    private lateinit var itemsAdapter: ContentItemsAdapter
    private var currentWorkspace: Workspace? = null
    private val args: Bundle? by lazy { arguments }
    private val workspaceId: String by lazy { args?.getString("workspaceId") ?: "" }

    private val pickImage = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        result.data?.data?.let { uri ->
            val imageItem = ContentItem.ImageItem(uri)
            currentWorkspace?.let { workspace ->
                workspaceRepository.addContentItem(workspace, imageItem)
            }
        }
    }

    private val pickFile = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        result.data?.data?.let { uri ->
            val fileName = getFileName(uri)
            val fileSize = getFileSize(uri)
            val fileItem = ContentItem.FileItem(fileName, uri, fileSize)
            currentWorkspace?.let { workspace ->
                workspaceRepository.addContentItem(workspace, fileItem)
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_workspace_detail, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        viewModel = ViewModelProvider(requireActivity())[MainViewModel::class.java]

        workspaceRepository = WorkspaceRepository.getInstance(requireContext())

        // Инициализация RecyclerView
        val recyclerView = view.findViewById<RecyclerView>(R.id.items_list)
        itemsAdapter = ContentItemsAdapter(
            onItemClick = { item ->
                when (item) {
                    is ContentItem.SubWorkspaceLink -> {
                        // Открываем вложенное пространство
                        navigateToWorkspace(item.workspaceId)
                    }
                    else -> {
                        // Обработка клика по другим типам элементов
                    }
                }
            },
            onItemLongClick = { _, _ ->
                // Handle long click
            }
        )
        recyclerView.apply {
            layoutManager = LinearLayoutManager(context)
            adapter = itemsAdapter
        }

        // Загрузка пространства
        loadWorkspace()

        // Настройка кнопки добавления
        view.findViewById<FloatingActionButton>(R.id.fab_add).setOnClickListener {
            showAddItemMenu(it)
        }

        // Observe workspace changes
        viewModel.workspaces.observe(viewLifecycleOwner) { workspaces ->
            currentWorkspace = workspaces.find { it.id == workspaceId }
            currentWorkspace?.let { workspace ->
                itemsAdapter.submitList(workspace.items)
            }
        }
    }

    private fun loadWorkspace() {
        currentWorkspace = workspaceRepository.getWorkspaceById(workspaceId)
        currentWorkspace?.let { workspace ->
            // Обновляем заголовок
            view?.findViewById<ImageView>(R.id.workspace_icon)?.setImageURI(workspace.iconUri)
            view?.findViewById<TextView>(R.id.workspace_title)?.text = workspace.name

            // Обновляем хлебные крошки
            updateBreadcrumbs(workspace)

            // Обновляем список элементов
            itemsAdapter.submitList(workspace.items)
        }
    }

    private fun updateBreadcrumbs(workspace: Workspace) {
        val breadcrumbs = mutableListOf<Workspace>()
        var current: Workspace? = workspace
        while (current != null) {
            breadcrumbs.add(0, current)
            current = current.parentId?.let { workspaceRepository.getWorkspaceById(it) }
        }

        view?.findViewById<ChipGroup>(R.id.breadcrumbs)?.apply {
            removeAllViews()
            breadcrumbs.forEach { ws ->
                addView(createBreadcrumbChip(ws))
            }
        }
    }

    private fun createBreadcrumbChip(workspace: Workspace): Chip {
        return Chip(requireContext()).apply {
            text = workspace.name
            isClickable = true
            setOnClickListener {
                navigateToWorkspace(workspace.id)
            }
        }
    }

    private fun showAddItemMenu(view: View) {
        val popup = PopupMenu(requireContext(), view)
        popup.menuInflater.inflate(R.menu.menu_add_item, popup.menu)
        
        popup.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.menu_add_text -> {
                    addTextItem()
                    true
                }
                R.id.menu_add_checkbox -> {
                    addCheckboxItem()
                    true
                }
                R.id.menu_add_image -> {
                    pickImage.launch(Intent(Intent.ACTION_PICK).apply {
                        type = "image/*"
                    })
                    true
                }
                R.id.menu_add_file -> {
                    pickFile.launch(Intent(Intent.ACTION_GET_CONTENT).apply {
                        type = "*/*"
                        addCategory(Intent.CATEGORY_OPENABLE)
                    })
                    true
                }
                R.id.menu_add_numbered_list -> {
                    addNumberedListItem()
                    true
                }
                R.id.menu_add_bullet_list -> {
                    addBulletListItem()
                    true
                }
                R.id.menu_add_subworkspace -> {
                    addSubWorkspaceLink()
                    true
                }
                else -> false
            }
        }
        popup.show()
    }

    private fun addTextItem() {
        currentWorkspace?.let { workspace ->
            val textItem = ContentItem.TextItem("")
            workspaceRepository.addContentItem(workspace, textItem)
        }
    }

    private fun addCheckboxItem() {
        currentWorkspace?.let { workspace ->
            val checkboxItem = ContentItem.CheckboxItem("", false)
            workspaceRepository.addContentItem(workspace, checkboxItem)
        }
    }

    private fun addNumberedListItem() {
        currentWorkspace?.let { workspace ->
            val numberedListItem = ContentItem.NumberedListItem("", workspace.items.size + 1)
            workspaceRepository.addContentItem(workspace, numberedListItem)
        }
    }

    private fun addBulletListItem() {
        currentWorkspace?.let { workspace ->
            val bulletListItem = ContentItem.BulletListItem("")
            workspaceRepository.addContentItem(workspace, bulletListItem)
        }
    }

    private fun addSubWorkspaceLink() {
        currentWorkspace?.let { workspace ->
            val subWorkspaceLink = ContentItem.SubWorkspaceLink("", "")
            workspaceRepository.addContentItem(workspace, subWorkspaceLink)
        }
    }

    private fun navigateToWorkspace(workspaceId: String) {
        val bundle = Bundle().apply {
            putString("workspaceId", workspaceId)
        }
        findNavController().navigate(R.id.workspaceDetailFragment, bundle)
    }

    private fun getFileName(uri: Uri): String {
        return uri.lastPathSegment ?: "Unknown file"
    }

    private fun getFileSize(uri: Uri): Long {
        return try {
            requireContext().contentResolver.openFileDescriptor(uri, "r")?.statSize ?: 0L
        } catch (e: Exception) {
            0L
        }
    }

    private fun saveWorkspaceChanges() {
        currentWorkspace?.let { workspace ->
            workspaceRepository.updateWorkspace(workspace)
        }
    }

    override fun onPause() {
        super.onPause()
        saveWorkspaceChanges()
    }
} 