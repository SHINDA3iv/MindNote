from rest_framework import viewsets, permissions, status
from rest_framework.response import Response
from rest_framework.decorators import action
from rest_framework.views import APIView
from workspaces.models import (
    Workspace, Page, ImageElement, FileElement,
    CheckboxElement, TextElement, LinkElement
)
from workspaces.local_storage import LocalStorageManager
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

    def perform_create(self, serializer):
        workspace = serializer.save(author=self.request.user)
        # Сохраняем рабочее пространство локально
        storage = LocalStorageManager(self.request.user)
        storage.save_workspace(serializer.data)

    def perform_update(self, serializer):
        workspace = serializer.save()
        # Обновляем локальное хранилище
        storage = LocalStorageManager(self.request.user)
        storage.save_workspace(serializer.data)

    def perform_destroy(self, instance):
        # Удаляем из локального хранилища
        storage = LocalStorageManager(self.request.user)
        storage.delete_workspace(instance.id)
        instance.delete()


class PageViewSet(viewsets.ModelViewSet):
    queryset = Page.objects.all()
    serializer_class = PageSerializer
    permission_classes = [permissions.IsAuthenticated]

    def get_queryset(self):
        return Page.objects.filter(space__author=self.request.user)

    def perform_create(self, serializer):
        page = serializer.save()
        # Обновляем локальное хранилище
        storage = LocalStorageManager(self.request.user)
        workspace_data = WorkspaceSerializer(page.space).data
        storage.save_workspace(workspace_data)

    def perform_update(self, serializer):
        page = serializer.save()
        # Обновляем локальное хранилище
        storage = LocalStorageManager(self.request.user)
        workspace_data = WorkspaceSerializer(page.space).data
        storage.save_workspace(workspace_data)

    def perform_destroy(self, instance):
        if instance.is_main:
            raise serializers.ValidationError("Нельзя удалить главную страницу.")
        # Обновляем локальное хранилище
        storage = LocalStorageManager(self.request.user)
        workspace_data = WorkspaceSerializer(instance.space).data
        storage.save_workspace(workspace_data)
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

class SyncView(APIView):
    permission_classes = [permissions.IsAuthenticated]

    def post(self, request):
        """
        Обработка синхронизации данных рабочего пространства между клиентом и сервером
        """
        try:
            # Получаем данные рабочего пространства из запроса
            workspace_data = request.data.get('workspace')
            if not workspace_data:
                return Response(
                    {"detail": "No workspace data provided"},
                    status=status.HTTP_400_BAD_REQUEST
                )

            # Получаем или создаем рабочее пространство
            workspace, created = Workspace.objects.get_or_create(
                id=workspace_data.get('id'),
                defaults={
                    'title': workspace_data.get('title'),
                    'author': request.user
                }
            )

            # Обновляем рабочее пространство, если оно существует
            if not created:
                workspace.title = workspace_data.get('title', workspace.title)
                workspace.save()

            # Синхронизируем страницы
            pages_data = workspace_data.get('pages', [])
            for page_data in pages_data:
                page, created = Page.objects.get_or_create(
                    id=page_data.get('id'),
                    defaults={
                        'title': page_data.get('title'),
                        'space': workspace,
                        'is_main': page_data.get('is_main', False)
                    }
                )

                if not created:
                    page.title = page_data.get('title', page.title)
                    page.is_main = page_data.get('is_main', page.is_main)
                    page.save()

            # Сохраняем в локальное хранилище
            storage = LocalStorageManager(request.user)
            storage.save_workspace(workspace_data)

            # Возвращаем обновленные данные рабочего пространства
            serializer = WorkspaceSerializer(workspace)
            return Response(serializer.data)

        except Exception as e:
            return Response(
                {"detail": str(e)},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )


class GuestWorkspaceViewSet(viewsets.ModelViewSet):
    serializer_class = WorkspaceSerializer
    permission_classes = [permissions.AllowAny]

    def get_queryset(self):
        return []

    def perform_create(self, serializer):
        workspace = serializer.save()
        # Сохраняем рабочее пространство локально для гостя
        storage = LocalStorageManager()
        storage.save_workspace(serializer.data)

    def perform_update(self, serializer):
        workspace = serializer.save()
        # Обновляем локальное хранилище гостя
        storage = LocalStorageManager()
        storage.save_workspace(serializer.data)

    def perform_destroy(self, instance):
        # Удаляем из локального хранилища гостя
        storage = LocalStorageManager()
        storage.delete_workspace(instance.id)
        instance.delete()


class UserWorkspaceSyncView(APIView):
    permission_classes = [permissions.IsAuthenticated]

    def post(self, request):
        """
        Синхронизация рабочих пространств гостя с пользователем
        """
        try:
            should_copy = request.data.get('copy_guest_workspaces', False)
            storage = LocalStorageManager(request.user)

            if should_copy:
                # Копируем рабочие пространства гостя в папку пользователя
                storage.copy_guest_workspaces_to_user()

                # Получаем список всех рабочих пространств пользователя
                workspaces = storage.list_workspaces()

                # Проверяем конфликты с серверными рабочими пространствами
                server_workspaces = Workspace.objects.filter(author=request.user)
                conflicts = []

                for workspace in workspaces:
                    server_workspace = server_workspaces.filter(title=workspace['title']).first()
                    if server_workspace:
                        conflicts.append({
                            'local': workspace,
                            'server': WorkspaceSerializer(server_workspace).data
                        })

                return Response({
                    'workspaces': workspaces,
                    'conflicts': conflicts
                })
            else:
                # Просто загружаем рабочие пространства с сервера
                workspaces = Workspace.objects.filter(author=request.user)
                serializer = WorkspaceSerializer(workspaces, many=True)
                return Response(serializer.data)

        except Exception as e:
            return Response(
                {"detail": str(e)},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )


class LogoutView(APIView):
    permission_classes = [permissions.IsAuthenticated]

    def post(self, request):
        """
        Обработка выхода пользователя
        """
        try:
            # Синхронизируем все рабочие пространства с сервером
            storage = LocalStorageManager(request.user)
            workspaces = storage.list_workspaces()

            for workspace in workspaces:
                workspace_data = storage.load_workspace(workspace['id'])
                if workspace_data:
                    # Создаем или обновляем рабочее пространство на сервере
                    workspace_obj, created = Workspace.objects.get_or_create(
                        id=workspace_data['id'],
                        defaults={
                            'title': workspace_data['title'],
                            'author': request.user
                        }
                    )

                    if not created:
                        workspace_obj.title = workspace_data['title']
                        workspace_obj.save()

            # Очищаем локальное хранилище пользователя
            storage.clear_user_workspaces()

            return Response(status=status.HTTP_200_OK)

        except Exception as e:
            return Response(
                {"detail": str(e)},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )


class UserValidationView(APIView):
    permission_classes = [permissions.IsAuthenticated]

    def post(self, request):
        """
        Обработка ошибки валидации пользователя
        """
        try:
            # Очищаем локальное хранилище пользователя
            storage = LocalStorageManager(request.user)
            storage.clear_user_workspaces()

            return Response({
                'detail': 'User validation failed. Please login again.'
            }, status=status.HTTP_401_UNAUTHORIZED)

        except Exception as e:
            return Response(
                {"detail": str(e)},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )
    