# views.py
from rest_framework import viewsets, permissions, status, serializers
from rest_framework.response import Response
from rest_framework.decorators import action
from rest_framework.views import APIView
from rest_framework.permissions import IsAuthenticated
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
import base64
from django.core.files.base import ContentFile
import traceback
import logging

logger = logging.getLogger(__name__)

class WorkspaceViewSet(viewsets.ModelViewSet):
    queryset = Workspace.objects.all()
    serializer_class = WorkspaceSerializer
    permission_classes = [permissions.IsAuthenticated]
    lookup_field = 'title'

    def get_queryset(self):
        return Workspace.objects.filter(author=self.request.user)

    def perform_create(self, serializer):
        workspace = serializer.save(author=self.request.user)
        storage = LocalStorageManager(self.request.user)
        storage.save_workspace(serializer.data)

    def perform_update(self, serializer):
        workspace = serializer.save()
        storage = LocalStorageManager(self.request.user)
        storage.save_workspace(serializer.data)

    def perform_destroy(self, instance):
        storage = LocalStorageManager(self.request.user)
        storage.delete_workspace(instance.title)
        instance.delete()

    @action(detail=True, methods=['get'])
    def full_structure(self, request, title=None):
        """
        Возвращает полную структуру пространства, включая все страницы и элементы.
        """
        workspace = self.get_object()
        data = WorkspaceSerializer(workspace).data
        
        def add_subpages(pages):
            for page in pages:
                page_obj = Page.objects.get(title=page['title'], space=workspace)
                page['elements'] = {
                    'images': ImageElementSerializer(page_obj.imageelement_elements.all(), many=True).data,
                    'files': FileElementSerializer(page_obj.fileelement_elements.all(), many=True).data,
                    'checkboxes': CheckboxElementSerializer(page_obj.checkboxelement_elements.all(), many=True).data,
                    'texts': TextElementSerializer(page_obj.textelement_elements.all(), many=True).data,
                    'links': LinkElementSerializer(page_obj.linkelement_elements.all(), many=True).data,
                }
                subpages = page_obj.subpages.all()
                if subpages.exists():
                    page['pages'] = PageSerializer(subpages, many=True).data
                    add_subpages(page['pages'])

        pages = workspace.pages.filter(parent_page__isnull=True)
        if pages.exists():
            data['pages'] = PageSerializer(pages, many=True).data
            add_subpages(data['pages'])
        
        return Response(data)

    @action(detail=True, methods=['get'], url_path='pages')
    def pages(self, request, title=None):
        workspace = self.get_object()
        pages = workspace.pages.filter(parent_page__isnull=True)
        serializer = PageSerializer(pages, many=True)
        return Response(serializer.data)


class PageViewSet(viewsets.ModelViewSet):
    queryset = Page.objects.all()
    serializer_class = PageSerializer
    permission_classes = [permissions.IsAuthenticated]
    lookup_field = 'title'

    def get_queryset(self):
        return Page.objects.filter(space__author=self.request.user)

    def perform_create(self, serializer):
        page = serializer.save()
        storage = LocalStorageManager(self.request.user)
        workspace_data = WorkspaceSerializer(page.space).data
        storage.save_workspace(workspace_data)

    def perform_update(self, serializer):
        page = serializer.save()
        storage = LocalStorageManager(self.request.user)
        workspace_data = WorkspaceSerializer(page.space).data
        storage.save_workspace(workspace_data)

    def perform_destroy(self, instance):
        storage = LocalStorageManager(self.request.user)
        workspace_data = WorkspaceSerializer(instance.space).data
        storage.save_workspace(workspace_data)
        instance.delete()

    @action(detail=True, methods=['post'])
    def add_element(self, request, title=None):
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
    def update_element(self, request, title=None, element_id=None):
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
    def remove_element(self, request, title=None, element_id=None):
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
            print("Received data:", request.data)  # Отладочный вывод
            
            # Получаем данные рабочего пространства из запроса
            workspace_data = request.data
            if not workspace_data:
                return Response(
                    {"detail": "No workspace data provided"},
                    status=status.HTTP_400_BAD_REQUEST
                )

            # Проверяем наличие обязательных полей
            if 'title' not in workspace_data:
                return Response(
                    {"detail": "Workspace title is required"},
                    status=status.HTTP_400_BAD_REQUEST
                )

            print("Processing workspace:", workspace_data['title'])  # Отладочный вывод

            # Получаем или создаем рабочее пространство
            workspace, created = Workspace.objects.get_or_create(
                title=workspace_data['title'],
                defaults={
                    'author': request.user,
                    'status': workspace_data.get('status', 'not_started')
                }
            )

            print(f"Workspace {'created' if created else 'updated'}:", workspace.title)  # Отладочный вывод

            # Обновляем рабочее пространство, если оно существует
            if not created:
                workspace.status = workspace_data.get('status', workspace.status)
                workspace.save()

            # Синхронизируем страницы
            pages_data = workspace_data.get('pages', [])
            existing_pages = {page.title: page for page in workspace.pages.all()}

            print(f"Processing {len(pages_data)} pages")  # Отладочный вывод

            for page_data in pages_data:
                page_title = page_data.get('title')
                if not page_title:
                    continue

                if page_title in existing_pages:
                    page = existing_pages[page_title]
                    page.save()
                    print(f"Updated page: {page_title}")  # Отладочный вывод
                else:
                    page = Page.objects.create(
                        title=page_title,
                        space=workspace,
                        parent_page=page_data.get('parent_page')
                    )
                    print(f"Created page: {page_title}")  # Отладочный вывод

            # Сохраняем в локальное хранилище
            storage = LocalStorageManager(request.user)
            storage.save_workspace(workspace_data)

            # Возвращаем обновленные данные рабочего пространства
            serializer = WorkspaceSerializer(workspace)
            response_data = serializer.data
            print("Sending response:", response_data)  # Отладочный вывод
            return Response(response_data)

        except Exception as e:
            print("Error in sync:", str(e))  # Отладочный вывод
            return Response(
                {"detail": str(e)},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )


class GuestWorkspaceViewSet(viewsets.ModelViewSet):
    serializer_class = WorkspaceSerializer
    permission_classes = [permissions.AllowAny]
    lookup_field = 'title'

    def get_queryset(self):
        return []

    def perform_create(self, serializer):
        workspace = serializer.save(is_guest=True)
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
        storage.delete_workspace(instance.title)
        instance.delete()


def create_page_with_elements(page_data, workspace):
    from workspaces.models import Page, TextElement, CheckboxElement, FileElement, LinkElement
    page = Page.objects.create(
        title=page_data['title'],
        space=workspace,
        parent_page=page_data.get('parent_page')
    )
    # Обработка иконки
    icon_b64 = page_data.get('icon')
    if icon_b64:
        try:
            icon_data = base64.b64decode(icon_b64)
            page.icon.save(f'{page.title}_icon.png', ContentFile(icon_data), save=True)
        except Exception:
            pass
    # Элементы страницы
    for el in page_data.get('elements', []):
        el_type = el.get('type')
        if el_type == 'TextItem':
            TextElement.objects.create(page=page, content=el.get('content', ''))
        elif el_type == 'CheckboxItem':
            CheckboxElement.objects.create(page=page, text=el.get('label', ''), is_checked=el.get('checked', False))
        elif el_type == 'FileItem':
            FileElement.objects.create(page=page, file=el.get('filePath', ''))
        elif el_type == 'SubspaceLinkItem':
            subspace_title = el.get('subspaceTitle', '')
            linked_page = Page.objects.filter(space=workspace, title=subspace_title).first()
            if linked_page:
                LinkElement.objects.create(page=page, linked_page=linked_page)

    # Рекурсивно вложенные страницы
    for subpage in page_data.get('pages', []):
        create_page_with_elements(subpage, workspace)


class UserWorkspaceSyncView(APIView):
    permission_classes = [permissions.IsAuthenticated]

    def post(self, request):
        """
        Синхронизация рабочих пространств гостя с пользователем.
        Ожидает: {"local_workspaces": [ ... ]}
        Возвращает: {"new": [...], "conflicts": [...], "server_only": [...]}
        """
        try:
            local_workspaces = request.data.get('local_workspaces', [])
            user = request.user

            # Получаем все workspaces пользователя с сервера
            server_workspaces_qs = Workspace.objects.filter(author=user)
            server_workspaces = {ws.title: ws for ws in server_workspaces_qs}
            server_titles = set(server_workspaces.keys())
            local_titles = set(ws['title'] for ws in local_workspaces)

            # Новые локальные workspaces (есть только локально)
            new_workspaces = [ws for ws in local_workspaces if ws['title'] not in server_titles]

            # Только серверные workspaces (есть только на сервере)
            server_only = [WorkspaceSerializer(server_workspaces[title]).data for title in (server_titles - local_titles)]

            # Конфликты (есть и там, и там, но содержимое отличается)
            conflicts = []
            for ws in local_workspaces:
                title = ws['title']
                if title in server_workspaces:
                    server_ws_data = WorkspaceSerializer(server_workspaces[title]).data
                    # Сравниваем содержимое (можно по hash, можно по сериализованному json)
                    if ws != server_ws_data:
                        conflicts.append({
                            'title': title,
                            'local': ws,
                            'server': server_ws_data
                        })

            response_data = {
                'new': new_workspaces,
                'conflicts': conflicts,
                'server_only': server_only,
            }
            return Response(response_data)

        except Exception as e:
            return Response({
                'detail': str(e),
                'traceback': traceback.format_exc()
            }, status=status.HTTP_500_INTERNAL_SERVER_ERROR)

    def patch(self, request):
        """
        Применение решения пользователя по конфликтам и новым workspaces.
        Ожидает: {"resolve": [{"title": ..., "use": "local"/"server", "data": {...}}], "new": [ ... ]}
        Возвращает: итоговый список workspaces пользователя.
        """
        try:
            user = request.user
            resolve = request.data.get('resolve', [])
            new_workspaces = request.data.get('new', [])

            # Применяем решения по конфликтам
            for item in resolve:
                title = item['title']
                use = item['use']
                data = item.get('data')
                ws_obj = Workspace.objects.filter(author=user, title=title).first()
                if use == 'local' and data:
                    if ws_obj:
                        ws_obj.status = data.get('status', ws_obj.status)
                        ws_obj.save()
                        ws_obj.pages.all().delete()
                        for page_data in data.get('pages', []):
                            create_page_with_elements(page_data, ws_obj)
                    else:
                        ws_obj = Workspace.objects.create(
                            title=title,
                            author=user,
                            status=data.get('status', 'not_started')
                        )
                        for page_data in data.get('pages', []):
                            create_page_with_elements(page_data, ws_obj)
                # use == 'server': ничего не делаем

            # Добавляем новые workspaces
            for ws in new_workspaces:
                if not Workspace.objects.filter(author=user, title=ws['title']).exists():
                    ws_obj = Workspace.objects.create(
                        title=ws['title'],
                        author=user,
                        status=ws.get('status', 'not_started')
                    )
                    for page_data in ws.get('pages', []):
                        create_page_with_elements(page_data, ws_obj)

            # Возвращаем полный актуальный список workspaces пользователя
            workspaces = Workspace.objects.filter(author=user)
            serializer = WorkspaceSerializer(workspaces, many=True)
            logger.debug("Serialized workspace data: %s", serializer.data)
            print(serializer.data)
            return Response(serializer.data)

        except Exception as e:
            return Response({
                'detail': str(e),
                'traceback': traceback.format_exc()
            }, status=status.HTTP_500_INTERNAL_SERVER_ERROR)


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
                workspace_data = storage.load_workspace(workspace['title'])
                if workspace_data:
                    # Создаем или обновляем рабочее пространство на сервере
                    workspace_obj, created = Workspace.objects.get_or_create(
                        title=workspace_data['title'],
                        defaults={
                            'author': request.user,
                            'status': workspace_data.get('status', 'not_started')
                        }
                    )

                    if not created:
                        workspace_obj.status = workspace_data.get('status', workspace_obj.status)
                        workspace_obj.save()

                    # Синхронизируем страницы
                    existing_pages = {page.title: page for page in workspace_obj.pages.all()}
                    for page_data in workspace_data.get('pages', []):
                        if page_data['title'] in existing_pages:
                            page = existing_pages[page_data['title']]
                            page.save()
                        else:
                            Page.objects.create(
                                title=page_data['title'],
                                space=workspace_obj,
                                parent_page=page_data.get('parent_page')
                            )

            # Очищаем локальное хранилище пользователя
            storage.clear_user_workspaces()

            return Response(status=status.HTTP_204_NO_CONTENT)

        except Exception as e:
            return Response(
                {"detail": str(e)},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )
    
class SaveGuestWorkspacesView(APIView):
    permission_classes = [IsAuthenticated]

    def post(self, request):
        """
        Сохраняет рабочие пространства гостя в аккаунт пользователя.
        Ожидает JSON данных с локальными рабочими пространствами гостя.
        """
        try:
            local_workspaces = request.data.get('local_workspaces', [])
            user = request.user

            for ws_data in local_workspaces:
                workspace, created = Workspace.objects.get_or_create(
                    title=ws_data['title'],
                    defaults={
                        'author': user,
                        'status': ws_data.get('status', 'not_started')
                    }
                )
                if not created:
                    # Обновляем существующее рабочее пространство
                    workspace.status = ws_data.get('status', workspace.status)
                    workspace.save()

                # Рекурсивно создаем страницы и элементы
                self._create_pages(workspace, ws_data.get('pages', []))

            return Response({"detail": "Рабочие пространства успешно сохранены."})
        except Exception as e:
            return Response({"detail": str(e)}, status=500)

    def _create_pages(self, workspace, pages_data, parent_page=None):
        for page_data in pages_data:
            page = Page.objects.create(
                space=workspace,
                title=page_data['title'],
                parent_page=parent_page
            )

            # Создаем элементы страницы
            for el_type, el_list in page_data.get('elements', {}).items():
                for el in el_list:
                    if el_type == 'images':
                        ImageElement.objects.create(page=page, **el)
                    elif el_type == 'files':
                        FileElement.objects.create(page=page, **el)
                    elif el_type == 'checkboxes':
                        CheckboxElement.objects.create(page=page, **el)
                    elif el_type == 'texts':
                        TextElement.objects.create(page=page, **el)
                    elif el_type == 'links':
                        linked_page_title = el.pop('linked_page', None)
                        if linked_page_title:
                            linked_page = Page.objects.filter(space=workspace, title=linked_page_title).first()
                            if linked_page:
                                LinkElement.objects.create(page=page, linked_page=linked_page, **el)
                        else:
                            LinkElement.objects.create(page=page, **el)

            # Рекурсивно создаем подстраницы
            self._create_pages(workspace, page_data.get('pages', []), page)