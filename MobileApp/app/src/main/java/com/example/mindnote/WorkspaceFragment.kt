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

class WorkspaceFragment : Fragment() {

    private lateinit var workspaceName: String
    private lateinit var container: LinearLayout

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Получаем имя рабочего пространства из аргументов
        workspaceName = arguments?.getString("WORKSPACE_NAME") ?: "Рабочее пространство"

        val view = inflater.inflate(R.layout.fragment_workspace, container, false)

        // Установка заголовка
        (activity as MainActivity).supportActionBar?.title = workspaceName

        // Инициализация контейнера для динамических элементов
        this.container = view.findViewById(R.id.container)

        return view
    }

    // Метод для добавления текстового поля
    fun addTextField() {
        val editText = EditText(context)
        editText.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        container.addView(editText)
    }

    fun addCheckboxItem(itemText: String) {
        val checkboxItemView = layoutInflater.inflate(R.layout.checkbox_item, null)
        val checkbox = checkboxItemView.findViewById<CheckBox>(R.id.checkbox)
        val editText = checkboxItemView.findViewById<EditText>(R.id.editTextCheckbox)

        editText.setText(itemText)

        // Устанавливаем обработчик двойного нажатия для редактирования
        editText.setOnTouchListener { v, event ->
            if (event.action == MotionEvent.ACTION_UP) {
                if (event.eventTime - event.downTime < 300) {
                    editText.isEnabled = true // Позволяем редактировать текст
                }
            }
            false
        }

        // Устанавливаем обработчик долгого нажатия для удаления/переименования
        checkboxItemView.setOnLongClickListener {
            showPopupMenuForItem(checkboxItemView, itemText)
            true
        }

        // Добавляем элемент в ваш контейнер (например, LinearLayout)
        container.addView(checkboxItemView)
    }

    private fun showPopupMenuForItem(view: View, itemText: String) {
        val popupMenu = PopupMenu(requireContext(), view)
        popupMenu.menuInflater.inflate(R.menu.popup_menu_item, popupMenu.menu)

        popupMenu.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.menu_rename -> {
                    // Логика переименования
                    showRenameDialog(itemText)
                    true
                }
                R.id.menu_delete -> {
                    // Логика удаления
                    container.removeView(view)
                    true
                }
                else -> false
            }
        }

        popupMenu.show()
    }

    private fun showRenameDialog(currentText: String) {
        val builder = AlertDialog.Builder(requireContext())
        val editText = EditText(requireContext())
        editText.setText(currentText)

        builder.setTitle("Переименовать элемент")
            .setView(editText)
            .setPositiveButton("Сохранить") { dialog, _ ->
                val newText = editText.text.toString()
                // Обновите текст чекбокса
                // Здесь нужно обновить текст соответствующего элемента
                dialog.dismiss()
            }
            .setNegativeButton("Отмена") { dialog, _ -> dialog.cancel() }

        builder.show()
    }



    // Метод для добавления изображения
    fun addImageView(imageUri: Uri?) {
        val imageView = ImageView(context)
        imageView.setImageURI(imageUri) // Устанавливаем изображение по URI
        imageView.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        container.addView(imageView)
    }


    companion object {
        fun newInstance(workspaceName: String): WorkspaceFragment {
            val fragment = WorkspaceFragment()
            val args = Bundle()
            args.putString("WORKSPACE_NAME", workspaceName)
            fragment.arguments = args
            return fragment
        }
    }
}
