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
import com.google.android.material.popupmenu.PopupMenu
import com.google.android.material.textfield.TextInputEditText

class WorkspaceDetailFragment : Fragment() {
    private lateinit var viewModel: MainViewModel
    private lateinit var workspaceRepository: WorkspaceRepository
    private lateinit var itemsAdapter: ContentItemsAdapter
    private var currentWorkspace: Workspace? = null
    private val args: WorkspaceDetailFragmentArgs by navArgs()

    private val pickImage = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        result.data?.data?.let { uri ->
            val imageItem = ContentItem.ImageItem(uri)
            currentWorkspace?.let { workspace ->
                viewModel.addContentItem(workspace, imageItem)
            }
        }
    }

    private val pickFile = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        result.data?.data?.let { uri ->
            val fileName = getFileName(uri)
            val fileSize = getFileSize(uri)
            val fileItem = ContentItem.FileItem(fileName, uri, fileSize)
            currentWorkspace?.let { workspace ->
                viewModel.addContentItem(workspace, fileItem)
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
            currentWorkspace = workspaces.find { it.id == args.workspaceId }
        }
    }

    private fun loadWorkspace() {
        currentWorkspace = workspaceRepository.getWorkspaceById(args.workspaceId)
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
            viewModel.addContentItem(workspace, textItem)
        }
    }

    private fun addCheckboxItem() {
        currentWorkspace?.let { workspace ->
            val checkboxItem = ContentItem.CheckboxItem("", false)
            viewModel.addContentItem(workspace, checkboxItem)
        }
    }

    private fun addNumberedListItem() {
        currentWorkspace?.let { workspace ->
            val numberedListItem = ContentItem.NumberedListItem("", workspace.items.size + 1)
            viewModel.addContentItem(workspace, numberedListItem)
        }
    }

    private fun addBulletListItem() {
        currentWorkspace?.let { workspace ->
            val bulletListItem = ContentItem.BulletListItem("")
            viewModel.addContentItem(workspace, bulletListItem)
        }
    }

    private fun addSubWorkspaceLink() {
        currentWorkspace?.let { workspace ->
            val subWorkspaceLink = ContentItem.SubWorkspaceLink("", "")
            viewModel.addContentItem(workspace, subWorkspaceLink)
        }
    }

    private fun navigateToWorkspace(workspaceId: String) {
        val action = WorkspaceDetailFragmentDirections
            .actionWorkspaceDetailSelf(workspaceId)
        findNavController().navigate(action)
    }

    private fun getFileName(uri: Uri): String {
        return requireContext().contentResolver.query(uri, null, null, null, null)?.use { cursor ->
            val nameIndex = cursor.getColumnIndex(android.provider.OpenableColumns.DISPLAY_NAME)
            cursor.moveToFirst()
            cursor.getString(nameIndex)
        } ?: "Unknown file"
    }

    private fun getFileSize(uri: Uri): Long {
        return requireContext().contentResolver.query(uri, null, null, null, null)?.use { cursor ->
            val sizeIndex = cursor.getColumnIndex(android.provider.OpenableColumns.SIZE)
            cursor.moveToFirst()
            cursor.getLong(sizeIndex)
        } ?: 0L
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