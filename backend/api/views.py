from rest_framework import viewsets
from rest_framework.response import Response
from workspaces.models import Workspace, Page
from .serializers import WorkspaceSerializer, PageSerializer
from rest_framework.decorators import action
from django.shortcuts import get_object_or_404

class WorkspaceViewSet(viewsets.ModelViewSet):
    queryset = Workspace.objects.all()
    serializer_class = WorkspaceSerializer

    def perform_create(self, serializer):
        serializer.save(owner=self.request.user)

    
class PageViewSet(viewsets.ModelViewSet):
    queryset = Page.objects.all()
    serializer_class = PageSerializer
    lookup_field = 'slug' 

    @action(detail=False, methods=['get'], url_path='(?P<slug>[-\w]+)')
    def by_slug(self, request, slug=None):
        page = get_object_or_404(Page, slug=slug)
        serializer = self.get_serializer(page)
        return Response(serializer.data)