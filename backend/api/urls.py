# urls.py
from django.urls import include, path
from rest_framework.routers import DefaultRouter

from .views import *

app_name = "api"

router = DefaultRouter()
# router.register("users", CustomUserViewSet, basename="users")
router.register(r'workspaces', WorkspaceViewSet, basename='workspace')
router.register(r'pages', PageViewSet, basename='page')

urlpatterns = [
    path("", include(router.urls)),
    path('', include('djoser.urls')),
    path('auth/', include('djoser.urls.authtoken')),
]
