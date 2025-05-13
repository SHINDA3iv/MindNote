package com.example.mindnote

import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.CheckBox
import android.widget.EditText
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.PopupMenu
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.example.mindnote.data.entity.PageElement
import com.example.mindnote.data.entity.ElementType
import com.example.mindnote.viewmodel.MindNoteViewModel

class WorkspaceFragment : Fragment() {
    private lateinit var workspaceId: String
    private lateinit var container: LinearLayout
    private lateinit var viewModel: MindNoteViewModel

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        workspaceId = arguments?.getString("WORKSPACE_ID") ?: "0"
        
        val view = inflater.inflate(R.layout.fragment_workspace, container, false)
        this.container = view.findViewById(R.id.container)
        
        viewModel = ViewModelProvider(requireActivity())[MindNoteViewModel::class.java]
        
        // Observe page elements
        viewModel.getElementsForPage(workspaceId.toLong()).observe(viewLifecycleOwner) { elements ->
            updatePageElements(elements)
        }
        
        return view
    }

    private fun updatePageElements(elements: List<PageElement>) {
        container.removeAllViews()
        elements.forEach { element ->
            when (element.type) {
                ElementType.TEXT -> addTextField(element)
                ElementType.CHECKBOX -> addCheckboxItem(element)
                ElementType.IMAGE -> addImageView(element)
                ElementType.VIDEO -> addVideoView(element)
                ElementType.FILE -> addFileView(element)
                ElementType.PAGE_LINK -> addPageLink(element)
            }
        }
    }

    fun addTextField(element: PageElement? = null) {
        val editText = EditText(context)
        editText.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            element?.let {
                x = it.x.toFloat()
                y = it.y.toFloat()
                width = it.width
                height = it.height
            }
        }
        
        if (element == null) {
            val newElement = PageElement(
                pageId = workspaceId.toLong(),
                type = ElementType.TEXT,
                content = "",
                x = 0,
                y = container.childCount * 100,
                width = LinearLayout.LayoutParams.MATCH_PARENT,
                height = LinearLayout.LayoutParams.WRAP_CONTENT
            )
            viewModel.createElement(newElement)
        } else {
            editText.setText(element.content)
        }
        
        container.addView(editText)
    }

    fun addCheckboxItem(element: PageElement? = null) {
        val checkboxItemView = layoutInflater.inflate(R.layout.checkbox_item, null)
        val checkbox = checkboxItemView.findViewById<CheckBox>(R.id.checkbox)
        val editText = checkboxItemView.findViewById<EditText>(R.id.editTextCheckbox)

        if (element == null) {
            val newElement = PageElement(
                pageId = workspaceId.toLong(),
                type = ElementType.CHECKBOX,
                content = "",
                x = 0,
                y = container.childCount * 100,
                width = LinearLayout.LayoutParams.MATCH_PARENT,
                height = LinearLayout.LayoutParams.WRAP_CONTENT
            )
            viewModel.createElement(newElement)
        } else {
            editText.setText(element.content)
        }

        editText.setOnTouchListener { v, event ->
            if (event.action == MotionEvent.ACTION_UP) {
                if (event.eventTime - event.downTime < 300) {
                    editText.isEnabled = true
                }
            }
            false
        }

        checkboxItemView.setOnLongClickListener {
            showPopupMenuForItem(checkboxItemView, element)
            true
        }

        container.addView(checkboxItemView)
    }

    fun addImageView(imageUri: Uri?) {
        if (imageUri != null) {
            val newElement = PageElement(
                pageId = workspaceId.toLong(),
                type = ElementType.IMAGE,
                content = imageUri.toString(),
                x = 0,
                y = container.childCount * 100,
                width = LinearLayout.LayoutParams.MATCH_PARENT,
                height = LinearLayout.LayoutParams.WRAP_CONTENT
            )
            viewModel.createElement(newElement)
        }
    }

    private fun addImageView(element: PageElement) {
        val imageView = ImageView(context)
        imageView.setImageURI(Uri.parse(element.content))
        imageView.layoutParams = LinearLayout.LayoutParams(
            element.width,
            element.height
        ).apply {
            x = element.x.toFloat()
            y = element.y.toFloat()
        }
        container.addView(imageView)
    }

    private fun addVideoView(element: PageElement) {
        // TODO: Implement video view
    }

    private fun addFileView(element: PageElement) {
        // TODO: Implement file view
    }

    private fun addPageLink(element: PageElement) {
        // TODO: Implement page link
    }

    private fun showPopupMenuForItem(view: View, element: PageElement?) {
        val popupMenu = PopupMenu(requireContext(), view)
        popupMenu.menuInflater.inflate(R.menu.popup_menu_item, popupMenu.menu)

        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.menu_rename -> {
                    element?.let { showRenameDialog(it) }
                    true
                }
                R.id.menu_delete -> {
                    element?.let { viewModel.deleteElement(it) }
                    container.removeView(view)
                    true
                }
                else -> false
            }
        }

        popupMenu.show()
    }

    private fun showRenameDialog(element: PageElement) {
        val builder = AlertDialog.Builder(requireContext())
        val editText = EditText(requireContext())
        editText.setText(element.content)

        builder.setTitle("Переименовать элемент")
            .setView(editText)
            .setPositiveButton("Сохранить") { _, _ ->
                val newContent = editText.text.toString()
                element.content = newContent
                viewModel.updateElement(element)
            }
            .setNegativeButton("Отмена") { dialog, _ -> dialog.cancel() }

        builder.show()
    }

    companion object {
        fun newInstance(workspaceId: String): WorkspaceFragment {
            val fragment = WorkspaceFragment()
            val args = Bundle()
            args.putString("WORKSPACE_ID", workspaceId)
            fragment.arguments = args
            return fragment
        }
    }
}
