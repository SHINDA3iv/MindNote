# urls.py
from django.urls import include, path
from rest_framework.routers import DefaultRouter

from .views import (
    WorkspaceViewSet, PageViewSet, SyncView,
    GuestWorkspaceViewSet, UserWorkspaceSyncView, LogoutView,
    UserValidationView
)

app_name = "api"

router = DefaultRouter()
# router.register("users", CustomUserViewSet, basename="users")
router.register(r'workspaces', WorkspaceViewSet, basename='workspace')
router.register(r'guest-workspaces', GuestWorkspaceViewSet, basename='guest-workspace')
router.register(r'pages', PageViewSet, basename='page')

urlpatterns = [
    path("", include(router.urls)),
    path('', include('djoser.urls')),
    path('auth/', include('djoser.urls.authtoken')),
    path('sync/', SyncView.as_view(), name='sync'),
    path('user-sync/', UserWorkspaceSyncView.as_view(), name='user-sync'),
    path('logout/', LogoutView.as_view(), name='logout'),
    path('user-validation/', UserValidationView.as_view(), name='user-validation'),
]
