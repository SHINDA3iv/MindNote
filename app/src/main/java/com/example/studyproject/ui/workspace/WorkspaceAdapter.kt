package com.example.studyproject.ui.workspace

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.example.studyproject.data.model.Workspace
import com.example.studyproject.databinding.ItemWorkspaceBinding

class WorkspaceAdapter(
    private val onClick: (Workspace) -> Unit
) : ListAdapter<Workspace, WorkspaceAdapter.WorkspaceViewHolder>(WorkspaceDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): WorkspaceViewHolder {
        val binding = ItemWorkspaceBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return WorkspaceViewHolder(binding)
    }

    override fun onBindViewHolder(holder: WorkspaceViewHolder, position: Int) {
        val workspace = getItem(position)
        holder.bind(workspace, onClick)
    }

    class WorkspaceViewHolder(
        private val binding: ItemWorkspaceBinding
    ) : RecyclerView.ViewHolder(binding.root) {
        fun bind(workspace: Workspace, onClick: (Workspace) -> Unit) {
            binding.workspaceName.text = workspace.name
            binding.root.setOnClickListener { onClick(workspace) }
        }
    }

    class WorkspaceDiffCallback : DiffUtil.ItemCallback<Workspace>() {
        override fun areItemsTheSame(oldItem: Workspace, newItem: Workspace): Boolean {
            return oldItem.id == newItem.id
        }

        override fun areContentsTheSame(oldItem: Workspace, newItem: Workspace): Boolean {
            return oldItem == newItem
        }
    }
}