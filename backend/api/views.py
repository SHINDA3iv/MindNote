from rest_framework import viewsets, permissions, status
from rest_framework.response import Response
from rest_framework.decorators import action
from workspaces.models import Workspace, Page, Element
from .serializers import WorkspaceSerializer, PageSerializer, ElementSerializer

class WorkspaceViewSet(viewsets.ModelViewSet):
    queryset = Workspace.objects.all()
    serializer_class = WorkspaceSerializer
    # permission_classes = [permissions.IsAuthenticated]

    # def get_queryset(self):
    #     # Пользователь может видеть только свои пространства
    #     return Workspace.objects.filter(author=self.request.user)

    def perform_create(self, serializer):
        # Передаем пользователя в контекст для автоматической привязки автора
        serializer.save(author=self.request.user)


class PageViewSet(viewsets.ModelViewSet):
    queryset = Page.objects.all()
    serializer_class = PageSerializer
    # permission_classes = [permissions.IsAuthenticated]

    # def get_queryset(self):
    #     # Пользователь может видеть только страницы своих пространств
    #     return Page.objects.filter(space__author=self.request.user)

    def perform_destroy(self, instance):
        # Запрет на удаление главной страницы
        if instance.is_main:
            raise serializers.ValidationError("Нельзя удалить главную страницу.")
        instance.delete()

    @action(detail=True, methods=['post'])
    def add_element(self, request, pk=None):
        page = self.get_object()
        element_type = request.data.get('element_type')
        element_data = {}

        if element_type == 'image':
            element_data['image'] = request.data.get('image')
        elif element_type == 'file':
            element_data['file'] = request.data.get('file')
        elif element_type == 'checkbox':
            element_data['text'] = request.data.get('text', '')
            element_data['is_checked'] = request.data.get('is_checked', False)
        elif element_type == 'text':
            element_data['content'] = request.data.get('content')
        elif element_type == 'link':
            element_data['linked_page'] = request.data.get('linked_page')

        serializer = ElementSerializer(
            data={'element_type': element_type},
            context={'request': request, 'page': page, 'element_data': element_data}
        )
        serializer.is_valid(raise_exception=True)
        serializer.save()
        return Response(serializer.data, status=status.HTTP_201_CREATED)

    @action(detail=True, methods=['patch'], url_path='update-element/(?P<element_id>[^/.]+)')
    def update_element(self, request, pk=None, element_id=None):
        page = self.get_object()
        try:
            element = page.elements.get(id=element_id)
        except Element.DoesNotExist:
            return Response({"detail": "Элемент не найден."}, status=status.HTTP_404_NOT_FOUND)

        element_data = {}
        if element.element_type == 'image':
            element_data['image'] = request.data.get('image')
        elif element.element_type == 'file':
            element_data['file'] = request.data.get('file')
        elif element.element_type == 'checkbox':
            element_data['text'] = request.data.get('text')
            element_data['is_checked'] = request.data.get('is_checked')
        elif element.element_type == 'text':
            element_data['content'] = request.data.get('content')
        elif element.element_type == 'link':
            element_data['linked_page'] = request.data.get('linked_page')

        serializer = ElementSerializer(
            element,
            data={'element_type': element.element_type},
            context={'request': request, 'element_data': element_data},
            partial=True
        )
        serializer.is_valid(raise_exception=True)
        serializer.save()
        return Response(serializer.data)

    @action(detail=True, methods=['delete'], url_path='remove-element/(?P<element_id>[^/.]+)')
    def remove_element(self, request, pk=None, element_id=None):
        page = self.get_object()
        try:
            element = page.elements.get(id=element_id)
        except Element.DoesNotExist:
            return Response({"detail": "Элемент не найден."}, status=status.HTTP_404_NOT_FOUND)
        element.delete()
        return Response(status=status.HTTP_204_NO_CONTENT)
    