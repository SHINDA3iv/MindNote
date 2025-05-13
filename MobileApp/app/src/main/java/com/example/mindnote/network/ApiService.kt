package com.example.mindnote.network

import com.example.mindnote.data.entity.Page
import com.example.mindnote.data.entity.PageElement
import com.example.mindnote.data.entity.Workspace
import retrofit2.Response
import retrofit2.http.*

interface ApiService {
    // Workspace endpoints
    @GET("workspaces")
    suspend fun getWorkspaces(): Response<List<Workspace>>

    @POST("workspaces")
    suspend fun createWorkspace(@Body workspace: Workspace): Response<Workspace>

    @PUT("workspaces/{id}")
    suspend fun updateWorkspace(@Path("id") id: Long, @Body workspace: Workspace): Response<Workspace>

    @DELETE("workspaces/{id}")
    suspend fun deleteWorkspace(@Path("id") id: Long): Response<Unit>

    // Page endpoints
    @GET("workspaces/{workspaceId}/pages")
    suspend fun getPages(@Path("workspaceId") workspaceId: Long): Response<List<Page>>

    @POST("workspaces/{workspaceId}/pages")
    suspend fun createPage(@Path("workspaceId") workspaceId: Long, @Body page: Page): Response<Page>

    @PUT("pages/{id}")
    suspend fun updatePage(@Path("id") id: Long, @Body page: Page): Response<Page>

    @DELETE("pages/{id}")
    suspend fun deletePage(@Path("id") id: Long): Response<Unit>

    // Page element endpoints
    @GET("pages/{pageId}/elements")
    suspend fun getPageElements(@Path("pageId") pageId: Long): Response<List<PageElement>>

    @POST("pages/{pageId}/elements")
    suspend fun createElement(@Path("pageId") pageId: Long, @Body element: PageElement): Response<PageElement>

    @PUT("elements/{id}")
    suspend fun updateElement(@Path("id") id: Long, @Body element: PageElement): Response<PageElement>

    @DELETE("elements/{id}")
    suspend fun deleteElement(@Path("id") id: Long): Response<Unit>

    // Sync endpoints
    @POST("sync")
    suspend fun syncData(@Body syncData: SyncData): Response<SyncResponse>
} 