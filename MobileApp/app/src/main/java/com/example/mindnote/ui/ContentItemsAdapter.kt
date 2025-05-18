package com.example.mindnote.ui

import android.content.Intent
import android.net.Uri
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.example.mindnote.R
import com.example.mindnote.data.ContentItem

class ContentItemsAdapter(
    private val onItemClick: (ContentItem) -> Unit,
    private val onItemLongClick: (View, ContentItem) -> Unit
) : ListAdapter<ContentItem, RecyclerView.ViewHolder>(ContentItemDiffCallback()) {

    override fun getItemViewType(position: Int): Int {
        return when (getItem(position)) {
            is ContentItem.TextItem -> VIEW_TYPE_TEXT
            is ContentItem.CheckboxItem -> VIEW_TYPE_CHECKBOX
            is ContentItem.NumberedListItem -> VIEW_TYPE_NUMBERED_LIST
            is ContentItem.BulletListItem -> VIEW_TYPE_BULLET_LIST
            is ContentItem.ImageItem -> VIEW_TYPE_IMAGE
            is ContentItem.FileItem -> VIEW_TYPE_FILE
            is ContentItem.SubWorkspaceLink -> VIEW_TYPE_SUBWORKSPACE
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        return when (viewType) {
            VIEW_TYPE_TEXT -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.text_item, parent, false)
                TextViewHolder(view)
            }
            VIEW_TYPE_CHECKBOX -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.checkbox_item, parent, false)
                CheckboxViewHolder(view)
            }
            VIEW_TYPE_NUMBERED_LIST -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.numbered_list_item, parent, false)
                NumberedListViewHolder(view)
            }
            VIEW_TYPE_BULLET_LIST -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.bullet_list_item, parent, false)
                BulletListViewHolder(view)
            }
            VIEW_TYPE_IMAGE -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.image_item, parent, false)
                ImageViewHolder(view)
            }
            VIEW_TYPE_FILE -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.file_item, parent, false)
                FileViewHolder(view)
            }
            VIEW_TYPE_SUBWORKSPACE -> {
                val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.subworkspace_link_item, parent, false)
                SubWorkspaceViewHolder(view)
            }
            else -> throw IllegalArgumentException("Unknown view type: $viewType")
        }
    }

    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
        val item = getItem(position)
        when (holder) {
            is TextViewHolder -> holder.bind(item as ContentItem.TextItem)
            is CheckboxViewHolder -> holder.bind(item as ContentItem.CheckboxItem)
            is NumberedListViewHolder -> holder.bind(item as ContentItem.NumberedListItem)
            is BulletListViewHolder -> holder.bind(item as ContentItem.BulletListItem)
            is ImageViewHolder -> holder.bind(item as ContentItem.ImageItem)
            is FileViewHolder -> holder.bind(item as ContentItem.FileItem)
            is SubWorkspaceViewHolder -> holder.bind(item as ContentItem.SubWorkspaceLink)
        }
    }

    inner class TextViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val editText: EditText = itemView.findViewById(R.id.editText)

        fun bind(item: ContentItem.TextItem) {
            editText.setText(item.text)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }
    }

    inner class CheckboxViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val checkbox: CheckBox = itemView.findViewById(R.id.checkbox)
        private val editText: EditText = itemView.findViewById(R.id.editTextCheckbox)

        fun bind(item: ContentItem.CheckboxItem) {
            checkbox.isChecked = item.isChecked
            editText.setText(item.text)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }
    }

    inner class NumberedListViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val numberText: TextView = itemView.findViewById(R.id.numberText)
        private val editText: EditText = itemView.findViewById(R.id.editTextListItem)

        fun bind(item: ContentItem.NumberedListItem) {
            numberText.text = item.number.toString()
            editText.setText(item.text)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }
    }

    inner class BulletListViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val editText: EditText = itemView.findViewById(R.id.editTextListItem)

        fun bind(item: ContentItem.BulletListItem) {
            editText.setText(item.text)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }
    }

    inner class ImageViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val imageView: ImageView = itemView.findViewById(R.id.imageView)

        fun bind(item: ContentItem.ImageItem) {
            imageView.setImageURI(item.imageUri)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }
    }

    inner class FileViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val fileNameText: TextView = itemView.findViewById(R.id.fileNameText)
        private val fileSizeText: TextView = itemView.findViewById(R.id.fileSizeText)

        fun bind(item: ContentItem.FileItem) {
            fileNameText.text = item.fileName
            fileSizeText.text = formatFileSize(item.fileSize)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }

        private fun formatFileSize(size: Long): String {
            return when {
                size < 1024 -> "$size B"
                size < 1024 * 1024 -> "${size / 1024} KB"
                size < 1024 * 1024 * 1024 -> "${size / (1024 * 1024)} MB"
                else -> "${size / (1024 * 1024 * 1024)} GB"
            }
        }
    }

    inner class SubWorkspaceViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val editText: EditText = itemView.findViewById(R.id.editTextLink)

        fun bind(item: ContentItem.SubWorkspaceLink) {
            editText.setText(item.displayName)
            itemView.setOnClickListener { onItemClick(item) }
            itemView.setOnLongClickListener { onItemLongClick(itemView, item); true }
        }
    }

    companion object {
        private const val VIEW_TYPE_TEXT = 0
        private const val VIEW_TYPE_CHECKBOX = 1
        private const val VIEW_TYPE_NUMBERED_LIST = 2
        private const val VIEW_TYPE_BULLET_LIST = 3
        private const val VIEW_TYPE_IMAGE = 4
        private const val VIEW_TYPE_FILE = 5
        private const val VIEW_TYPE_SUBWORKSPACE = 6
    }
}

class ContentItemDiffCallback : DiffUtil.ItemCallback<ContentItem>() {
    override fun areItemsTheSame(oldItem: ContentItem, newItem: ContentItem): Boolean {
        return oldItem.id == newItem.id
    }

    override fun areContentsTheSame(oldItem: ContentItem, newItem: ContentItem): Boolean {
        return oldItem == newItem
    }
} 