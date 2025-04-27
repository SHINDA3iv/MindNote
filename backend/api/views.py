from rest_framework import viewsets, permissions, status
from rest_framework.response import Response
from rest_framework.decorators import action
from workspaces.models import (
    Workspace, Page, ImageElement, FileElement,
    CheckboxElement, TextElement, LinkElement
)
from .serializers import (
    WorkspaceSerializer, PageSerializer, ImageElementSerializer,
    FileElementSerializer, CheckboxElementSerializer, TextElementSerializer,
    LinkElementSerializer
)


class WorkspaceViewSet(viewsets.ModelViewSet):
    queryset = Workspace.objects.all()
    serializer_class = WorkspaceSerializer
    permission_classes = [permissions.IsAuthenticated]

    def get_queryset(self):
        return Workspace.objects.filter(author=self.request.user)


class PageViewSet(viewsets.ModelViewSet):
    queryset = Page.objects.all()
    serializer_class = PageSerializer
    permission_classes = [permissions.IsAuthenticated]

    def get_queryset(self):
        return Page.objects.filter(space__author=self.request.user)

    def perform_destroy(self, instance):
        if instance.is_main:
            raise serializers.ValidationError("Нельзя удалить главную страницу.")
        instance.delete()

    @action(detail=True, methods=['post'])
    def add_element(self, request, pk=None):
        page = self.get_object()
        element_type = request.data.get('element_type')
        
        if element_type == 'image':
            serializer = ImageElementSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save(page=page)
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
            
        elif element_type == 'file':
            serializer = FileElementSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save(page=page)
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
            
        elif element_type == 'checkbox':
            serializer = CheckboxElementSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save(page=page)
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
            
        elif element_type == 'text':
            serializer = TextElementSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save(page=page)
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
            
        elif element_type == 'link':
            serializer = LinkElementSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save(page=page)
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
            
        return Response(
            {"detail": "Неизвестный тип элемента."},
            status=status.HTTP_400_BAD_REQUEST
        )

    @action(detail=True, methods=['patch'], url_path='update-element/(?P<element_id>[^/.]+)')
    def update_element(self, request, pk=None, element_id=None):
        page = self.get_object()
        element_type = request.data.get('element_type')
        
        try:
            if element_type == 'image':
                element = ImageElement.objects.get(id=element_id, page=page)
                serializer = ImageElementSerializer(element, data=request.data, partial=True)
            elif element_type == 'file':
                element = FileElement.objects.get(id=element_id, page=page)
                serializer = FileElementSerializer(element, data=request.data, partial=True)
            elif element_type == 'checkbox':
                element = CheckboxElement.objects.get(id=element_id, page=page)
                serializer = CheckboxElementSerializer(element, data=request.data, partial=True)
            elif element_type == 'text':
                element = TextElement.objects.get(id=element_id, page=page)
                serializer = TextElementSerializer(element, data=request.data, partial=True)
            elif element_type == 'link':
                element = LinkElement.objects.get(id=element_id, page=page)
                serializer = LinkElementSerializer(element, data=request.data, partial=True)
            else:
                return Response(
                    {"detail": "Неизвестный тип элемента."},
                    status=status.HTTP_400_BAD_REQUEST
                )
                
            if serializer.is_valid():
                serializer.save()
                return Response(serializer.data)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
            
        except (ImageElement.DoesNotExist, FileElement.DoesNotExist, 
                CheckboxElement.DoesNotExist, TextElement.DoesNotExist, 
                LinkElement.DoesNotExist):
            return Response(
                {"detail": "Элемент не найден."},
                status=status.HTTP_404_NOT_FOUND
            )

    @action(detail=True, methods=['delete'], url_path='remove-element/(?P<element_id>[^/.]+)')
    def remove_element(self, request, pk=None, element_id=None):
        page = self.get_object()
        element_type = request.data.get('element_type')
        
        try:
            if element_type == 'image':
                element = ImageElement.objects.get(id=element_id, page=page)
            elif element_type == 'file':
                element = FileElement.objects.get(id=element_id, page=page)
            elif element_type == 'checkbox':
                element = CheckboxElement.objects.get(id=element_id, page=page)
            elif element_type == 'text':
                element = TextElement.objects.get(id=element_id, page=page)
            elif element_type == 'link':
                element = LinkElement.objects.get(id=element_id, page=page)
            else:
                return Response(
                    {"detail": "Неизвестный тип элемента."},
                    status=status.HTTP_400_BAD_REQUEST
                )
                
            element.delete()
            return Response(status=status.HTTP_204_NO_CONTENT)
            
        except (ImageElement.DoesNotExist, FileElement.DoesNotExist, 
                CheckboxElement.DoesNotExist, TextElement.DoesNotExist, 
                LinkElement.DoesNotExist):
            return Response(
                {"detail": "Элемент не найден."},
                status=status.HTTP_404_NOT_FOUND
            )
    