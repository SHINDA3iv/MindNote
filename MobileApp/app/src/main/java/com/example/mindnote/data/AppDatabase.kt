package com.example.mindnote.data

import android.content.Context
import androidx.room.Database
import androidx.room.Room
import androidx.room.RoomDatabase
import androidx.room.TypeConverters
import com.example.mindnote.data.dao.PageDao
import com.example.mindnote.data.dao.PageElementDao
import com.example.mindnote.data.dao.WorkspaceDao
import com.example.mindnote.data.entity.Page
import com.example.mindnote.data.entity.PageElement
import com.example.mindnote.data.entity.Workspace

@Database(
    entities = [Workspace::class, Page::class, PageElement::class],
    version = 1,
    exportSchema = false
)
@TypeConverters(Converters::class)
abstract class AppDatabase : RoomDatabase() {
    abstract fun workspaceDao(): WorkspaceDao
    abstract fun pageDao(): PageDao
    abstract fun pageElementDao(): PageElementDao

    companion object {
        @Volatile
        private var INSTANCE: AppDatabase? = null

        fun getDatabase(context: Context): AppDatabase {
            return INSTANCE ?: synchronized(this) {
                val instance = Room.databaseBuilder(
                    context.applicationContext,
                    AppDatabase::class.java,
                    "mindnote_database"
                ).build()
                INSTANCE = instance
                instance
            }
        }
    }
} 