package com.example.mindnote

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageButton
import androidx.core.view.ViewCompat
import kotlin.math.abs

class SwipeContainer @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private var deleteButton: ImageButton
    private var itemContainer: FrameLayout
    private var initialX: Float = 0f
    private var currentX: Float = 0f
    private var isSwiping = false
    private var onDeleteListener: (() -> Unit)? = null
    private var isDeleteButtonVisible = false

    companion object {
        private const val SWIPE_THRESHOLD = 100f // Пороговое значение для начала свайпа
    }

    init {
        inflate(context, R.layout.swipe_container, this)
        deleteButton = findViewById(R.id.delete_button)
        itemContainer = findViewById(R.id.item_container)

        deleteButton.setOnClickListener {
            onDeleteListener?.invoke()
        }
    }

    fun setOnDeleteListener(listener: () -> Unit) {
        onDeleteListener = listener
    }

    fun addItemView(view: View) {
        itemContainer.removeAllViews()
        itemContainer.addView(view, LayoutParams(
            LayoutParams.MATCH_PARENT,
            LayoutParams.WRAP_CONTENT
        ))
        
        // Делаем view кликабельным и фокусируемым для корректной обработки касаний
        view.isClickable = true
        view.isFocusable = true
    }

    override fun onInterceptTouchEvent(ev: MotionEvent): Boolean {
        when (ev.action) {
            MotionEvent.ACTION_DOWN -> {
                initialX = ev.x
                currentX = initialX
                isSwiping = false
            }
            MotionEvent.ACTION_MOVE -> {
                val deltaX = ev.x - initialX
                if (abs(deltaX) > SWIPE_THRESHOLD) {
                    isSwiping = true
                }
            }
        }
        return isSwiping
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.action) {
            MotionEvent.ACTION_MOVE -> {
                if (isSwiping) {
                    val deltaX = event.x - initialX
                    if (isDeleteButtonVisible) {
                        // Если кнопка видна, позволяем свайп в обе стороны
                        currentX = (deleteButton.width + deltaX).coerceIn(0f, deleteButton.width.toFloat())
                    } else {
                        // Если кнопка скрыта, позволяем только свайп вправо
                        if (deltaX > 0) {
                            currentX = deltaX.coerceAtMost(deleteButton.width.toFloat())
                        } else {
                            currentX = 0f
                        }
                    }
                    itemContainer.translationX = currentX
                    deleteButton.visibility = View.VISIBLE
                    deleteButton.alpha = (currentX / deleteButton.width).coerceIn(0f, 1f)
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                if (isSwiping) {
                    if (isDeleteButtonVisible) {
                        // Если кнопка была видна, проверяем, нужно ли её скрыть
                        if (currentX < deleteButton.width / 2) {
                            // Скрываем кнопку
                            itemContainer.animate()
                                .translationX(0f)
                                .setDuration(150)
                                .withEndAction {
                                    deleteButton.visibility = View.GONE
                                    isDeleteButtonVisible = false
                                }
                                .start()
                        } else {
                            // Оставляем кнопку видимой
                            itemContainer.animate()
                                .translationX(deleteButton.width.toFloat())
                                .setDuration(150)
                                .start()
                        }
                    } else {
                        // Если кнопка была скрыта, проверяем, нужно ли её показать
                        if (currentX > deleteButton.width / 3) {
                            // Показываем кнопку
                            itemContainer.animate()
                                .translationX(deleteButton.width.toFloat())
                                .setDuration(150)
                                .withEndAction {
                                    isDeleteButtonVisible = true
                                }
                                .start()
                        } else {
                            // Скрываем кнопку
                            itemContainer.animate()
                                .translationX(0f)
                                .setDuration(150)
                                .withEndAction {
                                    deleteButton.visibility = View.GONE
                                }
                                .start()
                        }
                    }
                    isSwiping = false
                }
            }
        }
        return true
    }

    fun reset() {
        itemContainer.translationX = 0f
        deleteButton.visibility = View.GONE
        isDeleteButtonVisible = false
        isSwiping = false
    }
} 