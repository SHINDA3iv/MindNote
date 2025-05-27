import os
import shutil
import json
from pathlib import Path
from django.conf import settings
from django.core.files.storage import default_storage
from .models import Workspace, Page
from django.utils.text import slugify

class LocalStorageManager:
    def __init__(self, user=None):
        self.base_dir = Path(settings.BASE_DIR) / 'workspaces'
        self.user = user
        self.is_guest = user is None
        self.workspace_dir = self.base_dir / ('guest' if self.is_guest else str(user.id))
        self.ensure_directories()

    def ensure_directories(self):
        """Создает необходимые директории, если они не существуют"""
        self.workspace_dir.mkdir(parents=True, exist_ok=True)

    def get_workspace_path(self, title):
        """Возвращает путь к директории рабочего пространства"""
        # Используем slugify для безопасного имени файла
        safe_title = slugify(title)
        return self.workspace_dir / safe_title

    def save_workspace(self, workspace_data):
        title = workspace_data.get('title')
        workspace_path = self.get_workspace_path(title)
        workspace_path.mkdir(exist_ok=True)

        metadata = {
            'title': title,
            'created_at': workspace_data.get('created_at'),
            'status': workspace_data.get('status'),
            'pages': [],
            'elements': {
                'images': [],
                'files': [],
                'checkboxes': [],
                'texts': [],
                'links': []
            }
        }

        # Сохраняем элементы уровня пространства
        for el_type, el_list in workspace_data.get('elements', {}).items():
            for el in el_list:
                el_path = workspace_path / el_type
                el_path.mkdir(exist_ok=True)
                el_file_path = el_path / f"{uuid.uuid4()}.json"
                with open(el_file_path, 'w', encoding='utf-8') as f:
                    json.dump(el, f, ensure_ascii=False)
                metadata['elements'][el_type].append(str(el_file_path.relative_to(workspace_path)))

        # Рекурсивно сохраняем страницы
        def save_pages(pages_data, parent_path):
            for page_data in pages_data:
                page_title = page_data.get('title')
                page_path = parent_path / slugify(page_title)
                page_path.mkdir(exist_ok=True)

                page_metadata = {
                    'title': page_title,
                    'elements': {
                        'images': [],
                        'files': [],
                        'checkboxes': [],
                        'texts': [],
                        'links': []
                    },
                    'pages': []
                }

                # Сохраняем элементы страницы
                for el_type, el_list in page_data.get('elements', {}).items():
                    for el in el_list:
                        el_path = page_path / el_type
                        el_path.mkdir(exist_ok=True)
                        el_file_path = el_path / f"{uuid.uuid4()}.json"
                        with open(el_file_path, 'w', encoding='utf-8') as f:
                            json.dump(el, f, ensure_ascii=False)
                        page_metadata['elements'][el_type].append(str(el_file_path.relative_to(workspace_path)))

                # Рекурсивно сохраняем подстраницы
                subpages_data = page_data.get('pages', [])
                if subpages_data:
                    save_pages(subpages_data, page_path)
                    page_metadata['pages'] = [p['title'] for p in subpages_data]

                # Сохраняем метаданные страницы
                with open(page_path / 'metadata.json', 'w', encoding='utf-8') as f:
                    json.dump(page_metadata, f, ensure_ascii=False)

                metadata['pages'].append(page_metadata)

        save_pages(workspace_data.get('pages', []), workspace_path)

        # Сохраняем основные метаданные рабочего пространства
        with open(workspace_path / 'metadata.json', 'w', encoding='utf-8') as f:
            json.dump(metadata, f, ensure_ascii=False)

    def load_workspace(self, title):
        workspace_path = self.get_workspace_path(title)
        if not workspace_path.exists():
            return None

        with open(workspace_path / 'metadata.json', 'r', encoding='utf-8') as f:
            metadata = json.load(f)

        workspace_data = {
            'title': metadata['title'],
            'created_at': metadata['created_at'],
            'status': metadata['status'],
            'pages': [],
            'elements': {
                'images': [],
                'files': [],
                'checkboxes': [],
                'texts': [],
                'links': []
            }
        }

        # Загружаем элементы уровня пространства
        for el_type, el_paths in metadata['elements'].items():
            for el_path in el_paths:
                with open(workspace_path / el_path, 'r', encoding='utf-8') as f:
                    el_data = json.load(f)
                    workspace_data['elements'][el_type].append(el_data)

        # Рекурсивно загружаем страницы
        def load_pages(pages_data, parent_path):
            for page_data in pages_data:
                page_title = page_data['title']
                page_path = parent_path / slugify(page_title)

                with open(page_path / 'metadata.json', 'r', encoding='utf-8') as f:
                    page_metadata = json.load(f)

                loaded_page = {
                    'title': page_metadata['title'],
                    'elements': {
                        'images': [],
                        'files': [],
                        'checkboxes': [],
                        'texts': [],
                        'links': []
                    },
                    'pages': []
                }

                # Загружаем элементы страницы
                for el_type, el_paths in page_metadata['elements'].items():
                    for el_path in el_paths:
                        with open(workspace_path / el_path, 'r', encoding='utf-8') as f:
                            el_data = json.load(f)
                            loaded_page['elements'][el_type].append(el_data)

                # Рекурсивно загружаем подстраницы
                if page_metadata['pages']:
                    loaded_page['pages'] = load_pages(page_metadata['pages'], page_path)

                workspace_data['pages'].append(loaded_page)

            return pages_data

        load_pages(metadata['pages'], workspace_path)
        return workspace_data

    def list_workspaces(self):
        workspaces = []
        for workspace_dir in os.listdir(self.workspace_dir):
            workspace_path = self.workspace_dir / workspace_dir
            if workspace_path.is_dir():
                try:
                    with open(workspace_path / 'metadata.json', 'r', encoding='utf-8') as f:
                        metadata = json.load(f)
                        workspaces.append({
                            'title': metadata['title'],
                            'created_at': metadata['created_at'],
                            'status': metadata['status']
                        })
                except (FileNotFoundError, json.JSONDecodeError):
                    continue
        return workspaces

    def delete_workspace(self, title):
        """Удаляет рабочее пространство из локального хранилища"""
        workspace_path = self.get_workspace_path(title)
        if workspace_path.exists():
            shutil.rmtree(workspace_path)

    def copy_guest_workspaces_to_user(self):
        """Копирует все рабочие пространства гостя в папку пользователя"""
        if self.is_guest:
            raise ValueError("Cannot copy guest workspaces while in guest mode")
        guest_dir = self.base_dir / 'guest'
        user_dir = self.base_dir / str(self.user.id)
        if not guest_dir.exists():
            return
        for workspace_dir in os.listdir(guest_dir):
            guest_workspace_path = guest_dir / workspace_dir
            if guest_workspace_path.is_dir():
                try:
                    with open(guest_workspace_path / 'metadata.json', 'r', encoding='utf-8') as f:
                        metadata = json.load(f)
                        title = metadata['title']
                        user_workspace_path = user_dir / slugify(title)
                        shutil.copytree(guest_workspace_path, user_workspace_path, dirs_exist_ok=True)
                except (FileNotFoundError, json.JSONDecodeError):
                    continue

    def clear_user_workspaces(self):
        """Очищает все рабочие пространства пользователя"""
        if self.is_guest:
            raise ValueError("Cannot clear workspaces while in guest mode")
        
        if self.workspace_dir.exists():
            shutil.rmtree(self.workspace_dir)
        self.workspace_dir.mkdir()