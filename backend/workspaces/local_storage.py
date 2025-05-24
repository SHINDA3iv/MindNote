import os
import shutil
import json
from pathlib import Path
from django.conf import settings
from django.core.files.storage import default_storage
from .models import Workspace, Page

class LocalStorageManager:
    def __init__(self, user=None):
        self.base_dir = Path(settings.BASE_DIR) / 'workspaces'
        self.user = user
        self.is_guest = user is None
        self.workspace_dir = self.base_dir / ('guest' if self.is_guest else 'user')

    def ensure_directories(self):
        """Создает необходимые директории, если они не существуют"""
        self.workspace_dir.mkdir(parents=True, exist_ok=True)

    def get_workspace_path(self, workspace_id):
        """Возвращает путь к директории рабочего пространства"""
        return self.workspace_dir / str(workspace_id)

    def save_workspace(self, workspace_data):
        """Сохраняет рабочее пространство локально"""
        workspace_id = workspace_data.get('id')
        workspace_path = self.get_workspace_path(workspace_id)
        workspace_path.mkdir(exist_ok=True)

        # Сохраняем метаданные рабочего пространства
        metadata = {
            'title': workspace_data.get('title'),
            'created_at': workspace_data.get('created_at'),
            'status': workspace_data.get('status'),
            'pages': []
        }

        # Сохраняем страницы
        for page_data in workspace_data.get('pages', []):
            page_id = page_data.get('id')
            page_path = workspace_path / str(page_id)
            page_path.mkdir(exist_ok=True)

            # Сохраняем метаданные страницы
            page_metadata = {
                'title': page_data.get('title'),
                'is_main': page_data.get('is_main', False),
                'elements': []
            }

            # Сохраняем элементы страницы
            for element in page_data.get('elements', []):
                element_id = element.get('id')
                element_path = page_path / str(element_id)
                element_path.mkdir(exist_ok=True)

                # Сохраняем данные элемента
                with open(element_path / 'data.json', 'w', encoding='utf-8') as f:
                    json.dump(element, f, ensure_ascii=False)

                page_metadata['elements'].append({
                    'id': element_id,
                    'type': element.get('type')
                })

            metadata['pages'].append(page_metadata)

            # Сохраняем метаданные страницы
            with open(page_path / 'metadata.json', 'w', encoding='utf-8') as f:
                json.dump(page_metadata, f, ensure_ascii=False)

        # Сохраняем метаданные рабочего пространства
        with open(workspace_path / 'metadata.json', 'w', encoding='utf-8') as f:
            json.dump(metadata, f, ensure_ascii=False)

    def load_workspace(self, workspace_id):
        """Загружает рабочее пространство из локального хранилища"""
        workspace_path = self.get_workspace_path(workspace_id)
        if not workspace_path.exists():
            return None

        # Загружаем метаданные рабочего пространства
        with open(workspace_path / 'metadata.json', 'r', encoding='utf-8') as f:
            metadata = json.load(f)

        workspace_data = {
            'id': workspace_id,
            'title': metadata['title'],
            'created_at': metadata['created_at'],
            'status': metadata['status'],
            'pages': []
        }

        # Загружаем страницы
        for page_metadata in metadata['pages']:
            page_id = page_metadata['id']
            page_path = workspace_path / str(page_id)
            
            page_data = {
                'id': page_id,
                'title': page_metadata['title'],
                'is_main': page_metadata['is_main'],
                'elements': []
            }

            # Загружаем элементы страницы
            for element_metadata in page_metadata['elements']:
                element_id = element_metadata['id']
                element_path = page_path / str(element_id)

                with open(element_path / 'data.json', 'r', encoding='utf-8') as f:
                    element_data = json.load(f)
                    page_data['elements'].append(element_data)

            workspace_data['pages'].append(page_data)

        return workspace_data

    def list_workspaces(self):
        """Возвращает список всех локальных рабочих пространств"""
        workspaces = []
        for workspace_id in os.listdir(self.workspace_dir):
            workspace_path = self.get_workspace_path(workspace_id)
            if workspace_path.is_dir():
                try:
                    with open(workspace_path / 'metadata.json', 'r', encoding='utf-8') as f:
                        metadata = json.load(f)
                        workspaces.append({
                            'id': workspace_id,
                            'title': metadata['title'],
                            'created_at': metadata['created_at'],
                            'status': metadata['status']
                        })
                except (FileNotFoundError, json.JSONDecodeError):
                    continue
        return workspaces

    def delete_workspace(self, workspace_id):
        """Удаляет рабочее пространство из локального хранилища"""
        workspace_path = self.get_workspace_path(workspace_id)
        if workspace_path.exists():
            shutil.rmtree(workspace_path)

    def copy_guest_workspaces_to_user(self):
        """Копирует все рабочие пространства гостя в папку пользователя"""
        if self.is_guest:
            raise ValueError("Cannot copy guest workspaces while in guest mode")

        guest_dir = self.base_dir / 'guest'
        if not guest_dir.exists():
            return

        for workspace_id in os.listdir(guest_dir):
            guest_workspace_path = guest_dir / workspace_id
            user_workspace_path = self.get_workspace_path(workspace_id)
            
            if guest_workspace_path.is_dir():
                shutil.copytree(guest_workspace_path, user_workspace_path, dirs_exist_ok=True)

    def clear_user_workspaces(self):
        """Очищает все рабочие пространства пользователя"""
        if self.is_guest:
            raise ValueError("Cannot clear workspaces while in guest mode")

        if self.workspace_dir.exists():
            shutil.rmtree(self.workspace_dir)
            self.workspace_dir.mkdir()

    def validate_user_workspaces(self):
        """
        Проверяет валидность рабочих пространств пользователя.
        Возвращает True, если все в порядке, False если есть проблемы.
        """
        if self.is_guest:
            return True

        try:
            # Проверяем структуру директорий
            if not self.workspace_dir.exists():
                return False

            # Проверяем каждое рабочее пространство
            for workspace_id in os.listdir(self.workspace_dir):
                workspace_path = self.get_workspace_path(workspace_id)
                if not workspace_path.is_dir():
                    return False

                # Проверяем метаданные
                metadata_path = workspace_path / 'metadata.json'
                if not metadata_path.exists():
                    return False

                try:
                    with open(metadata_path, 'r', encoding='utf-8') as f:
                        metadata = json.load(f)
                        if not all(key in metadata for key in ['title', 'created_at', 'status', 'pages']):
                            return False
                except (json.JSONDecodeError, KeyError):
                    return False

            return True
        except Exception:
            return False 