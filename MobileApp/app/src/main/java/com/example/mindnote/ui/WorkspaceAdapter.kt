package com.example.mindnote.ui

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.example.mindnote.R
import com.example.mindnote.data.Workspace
import com.example.mindnote.data.ContentItem

class WorkspaceAdapter(
    private var workspaces: List<Workspace>,
    private val onExpandClick: (Workspace, Boolean) -> Unit,
    private val onWorkspaceClick: (Workspace) -> Unit
) : ListAdapter<Workspace, WorkspaceAdapter.WorkspaceViewHolder>(WorkspaceDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): WorkspaceViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.workspace_list_item, parent, false)
        return WorkspaceViewHolder(view)
    }

    override fun onBindViewHolder(holder: WorkspaceViewHolder, position: Int) {
        val workspace = getItem(position)
        holder.bind(workspace)
    }

    inner class WorkspaceViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val nameTextView: TextView = itemView.findViewById(R.id.workspace_name)
        private val iconImageView: ImageView = itemView.findViewById(R.id.workspace_icon)
        private val expandImageView: ImageView = itemView.findViewById(R.id.workspace_expand_icon)

        fun bind(workspace: Workspace) {
            nameTextView.text = workspace.name
            workspace.iconUri?.let { uri ->
                iconImageView.setImageURI(uri)
            } ?: run {
                iconImageView.setImageResource(R.drawable.ic_workspace_default)
            }

            // Show expand icon only if workspace has subworkspaces
            expandImageView.visibility = if (workspace.items.any { it is ContentItem.SubWorkspaceLink }) View.VISIBLE else View.GONE

            // Set click listeners
            itemView.setOnClickListener { onWorkspaceClick(workspace) }
            expandImageView.setOnClickListener {
                val isExpanded = expandImageView.rotation == 0f
                expandImageView.animate()
                    .rotation(if (isExpanded) 90f else 0f)
                    .setDuration(200)
                    .start()
                onExpandClick(workspace, isExpanded)
            }
        }
    }

    companion object {
        class WorkspaceDiffCallback : DiffUtil.ItemCallback<Workspace>() {
            override fun areItemsTheSame(oldItem: Workspace, newItem: Workspace): Boolean {
                return oldItem.id == newItem.id
            }

            override fun areContentsTheSame(oldItem: Workspace, newItem: Workspace): Boolean {
                return oldItem == newItem
            }
        }
    }
} 