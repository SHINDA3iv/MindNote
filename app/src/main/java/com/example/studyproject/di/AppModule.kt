package com.example.studyproject.di

import com.example.studyproject.data.repository.WorkspaceRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {

    @Provides
    @Singleton
    fun provideWorkspaceRepository(): WorkspaceRepository {
        return WorkspaceRepository()
    }
}
