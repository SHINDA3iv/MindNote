package com.example.studyproject.ui.workspace

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.os.bundleOf
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.navigation.fragment.findNavController
import com.example.studyproject.R
import com.example.studyproject.databinding.FragmentWorkspaceBinding
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class WorkspaceFragment : Fragment() {

    private val viewModel: WorkspaceViewModel by viewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val binding = FragmentWorkspaceBinding.inflate(inflater, container, false)
        val adapter = WorkspaceAdapter { workspace ->
            findNavController().navigate(
                R.id.action_workspaceFragment_to_editorFragment,
                bundleOf("workspaceId" to workspace.id)
            )
        }
        binding.recyclerView.adapter = adapter
        viewModel.workspaces.observe(viewLifecycleOwner) { workspaces ->
            adapter.submitList(workspaces)
        }

        binding.addWorkspaceButton.setOnClickListener {
            viewModel.createWorkspace("New Workspace")
        }
        return binding.root
    }
}