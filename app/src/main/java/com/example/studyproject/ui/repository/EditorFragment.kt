package com.example.studyproject.ui.repository

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.example.studyproject.data.model.BaseElement
import dagger.hilt.android.AndroidEntryPoint
import androidx.fragment.app.viewModels
import com.example.studyproject.databinding.FragmentEditorBinding

@AndroidEntryPoint
class EditorFragment : Fragment() {

    private val viewModel: EditorViewModel by viewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val binding = FragmentEditorBinding.inflate(inflater, container, false)
        val workspaceId = arguments?.getString("workspaceId") ?: return binding.root
        viewModel.loadWorkspace(workspaceId)

        binding.addElementButton.setOnClickListener {
            viewModel.addElement(BaseElement.TextElement("New Text"))
        }

        viewModel.workspace.observe(viewLifecycleOwner) { workspace ->
            // Обновить UI
        }
        return binding.root
    }
}
