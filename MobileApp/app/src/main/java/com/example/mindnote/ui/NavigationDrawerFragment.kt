package com.example.mindnote.ui

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.mindnote.MainActivity
import com.example.mindnote.R
import com.example.mindnote.data.Workspace
import com.example.mindnote.data.WorkspaceRepository
import com.google.android.material.button.MaterialButton
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.google.android.material.textfield.TextInputEditText

class NavigationDrawerFragment : Fragment() {
    private lateinit var workspaceRepository: WorkspaceRepository
    private lateinit var workspaceAdapter: WorkspaceAdapter
    private var selectedIconUri: Uri? = null
    private lateinit var viewModel: MainViewModel

    private val pickImage = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            result.data?.data?.let { uri ->
                selectedIconUri = uri
                // Обновляем иконку в диалоге
                dialogIconView?.setImageURI(uri)
            }
        }
    }

    private var dialogIconView: ImageView? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_navigation_drawer, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        viewModel = ViewModelProvider(requireActivity())[MainViewModel::class.java]

        workspaceRepository = WorkspaceRepository.getInstance(requireContext())

        // Инициализация RecyclerView
        val recyclerView = view.findViewById<RecyclerView>(R.id.workspaces_list)
        workspaceAdapter = WorkspaceAdapter(
            workspaces = emptyList(),
            onExpandClick = { workspace, isExpanded ->
                // Handle expand/collapse
                viewModel.toggleWorkspaceExpansion(workspace.id, isExpanded)
            },
            onWorkspaceClick = { workspace ->
                (activity as? MainActivity)?.openWorkspace(workspace)
            }
        )
        recyclerView.apply {
            layoutManager = LinearLayoutManager(context)
            adapter = workspaceAdapter
        }

        // Обработка кнопки создания пространства
        view.findViewById<MaterialButton>(R.id.button_create_workspace).setOnClickListener {
            showCreateWorkspaceDialog()
        }

        // Observe workspaces
        viewModel.workspaces.observe(viewLifecycleOwner) { workspaces ->
            workspaceAdapter.submitList(workspaces)
        }
    }

    fun showCreateWorkspaceDialog() {
        val dialog = AlertDialog.Builder(requireContext())
            .setTitle("Create Workspace")
            .setMessage("Enter workspace name:")
            .setPositiveButton("Create") { _, _ ->
                // Handle workspace creation
                viewModel.createWorkspace("New Workspace")
            }
            .setNegativeButton("Cancel", null)
            .create()
        dialog.show()
    }

    fun updateWorkspaceList(workspaces: List<Workspace>) {
        workspaceAdapter.submitList(workspaces)
    }

    companion object {
        fun newInstance() = NavigationDrawerFragment()
    }
} 