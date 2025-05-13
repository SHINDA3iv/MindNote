package com.example.mindnote.data

import androidx.room.TypeConverter
import com.example.mindnote.data.entity.ElementType

class Converters {
    @TypeConverter
    fun fromElementType(value: ElementType): String {
        return value.name
    }

    @TypeConverter
    fun toElementType(value: String): ElementType {
        return ElementType.valueOf(value)
    }
} 