from django.urls import path
from . import views

app_name = 'workspaces'

urlpatterns = [
    path('', views.WorkspaceListView.as_view(), name='workspace-list'),
    path('<int:pk>/', views.WorkspaceDetailView.as_view(), name='workspace-detail'),
    path('<int:workspace_id>/items/', views.WorkspaceItemListView.as_view(), name='workspace-item-list'),
    path('<int:workspace_id>/items/<int:pk>/', views.WorkspaceItemDetailView.as_view(), name='workspace-item-detail'),
] 