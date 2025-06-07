import base64
from django.core.files.base import ContentFile
from rest_framework import serializers
from workspaces.models import (
    Workspace, Page, Element, ImageElement,
    FileElement, CheckboxElement,
    TextElement, LinkElement
)
import logging
import pprint

logger = logging.getLogger(__name__)

class Base64ImageField(serializers.ImageField):
    def to_internal_value(self, data):
        if isinstance(data, str) and data.startswith("data:image"):
            format, imgstr = data.split(";base64,")
            ext = format.split("/")[-1]
            data = ContentFile(base64.b64decode(imgstr), name="temp." + ext)
        return super().to_internal_value(data)


class ImageElementSerializer(serializers.ModelSerializer):
    type = serializers.SerializerMethodField()
    imageData = serializers.SerializerMethodField()
    id = serializers.IntegerField(read_only=True)

    class Meta:
        model = ImageElement
        fields = ['id', 'type', 'imageData', 'created_at']
        read_only_fields = ['created_at']

    def get_type(self, obj):
        return 'ImageItem'

    def get_imageData(self, obj):
        if obj.image and hasattr(obj.image, 'url'):
            try:
                with obj.image.open('rb') as f:
                    return base64.b64encode(f.read()).decode('utf-8')
            except Exception:
                return ''
        return ''


class FileElementSerializer(serializers.ModelSerializer):
    type = serializers.SerializerMethodField()
    filePath = serializers.SerializerMethodField()
    id = serializers.IntegerField(read_only=True)

    class Meta:
        model = FileElement
        fields = ['id', 'type', 'filePath', 'created_at']
        read_only_fields = ['created_at']

    def get_type(self, obj):
        return 'FileItem'

    def get_filePath(self, obj):
        return obj.file.name if obj.file else ''


class CheckboxElementSerializer(serializers.ModelSerializer):
    type = serializers.SerializerMethodField()
    label = serializers.CharField(source='text')
    checked = serializers.BooleanField(source='is_checked')
    id = serializers.IntegerField(read_only=True)

    class Meta:
        model = CheckboxElement
        fields = ['id', 'type', 'label', 'checked', 'created_at']
        read_only_fields = ['created_at']

    def get_type(self, obj):
        return 'CheckboxItem'


class TextElementSerializer(serializers.ModelSerializer):
    type = serializers.SerializerMethodField()
    id = serializers.IntegerField(read_only=True)

    class Meta:
        model = TextElement
        fields = ['id', 'type', 'content', 'created_at']
        read_only_fields = ['created_at']

    def get_type(self, obj):
        return 'TextItem'


class LinkElementSerializer(serializers.ModelSerializer):
    type = serializers.SerializerMethodField()
    subspaceTitle = serializers.SerializerMethodField()
    id = serializers.IntegerField(read_only=True)

    class Meta:
        model = LinkElement
        fields = ['id', 'type', 'subspaceTitle', 'linked_page', 'created_at']
        read_only_fields = ['created_at']

    def get_type(self, obj):
        return 'SubspaceLinkItem'

    def get_subspaceTitle(self, obj):
        return obj.linked_page.title if obj.linked_page else None


class PageSerializer(serializers.ModelSerializer):
    space = serializers.SlugRelatedField(slug_field='title', queryset=Workspace.objects.all())
    elements = serializers.SerializerMethodField()
    icon = serializers.SerializerMethodField()
    pages = serializers.SerializerMethodField()

    class Meta:
        model = Page
        fields = [
            'title', 'space', 'elements', 'created_at', 'icon', 'pages'
        ]
        read_only_fields = ['created_at']

    def get_elements(self, obj):
        # Собираем все элементы, сортируем по created_at
        all_elements = []
        all_elements.extend(list(obj.imageelement_elements.all()))
        all_elements.extend(list(obj.fileelement_elements.all()))
        all_elements.extend(list(obj.checkboxelement_elements.all()))
        all_elements.extend(list(obj.textelement_elements.all()))
        all_elements.extend(list(obj.linkelement_elements.all()))
        all_elements.sort(key=lambda el: el.created_at)
        result = []
        for el in all_elements:
            if hasattr(el, 'image'):
                result.append(ImageElementSerializer(el).data)
            elif hasattr(el, 'file'):
                result.append(FileElementSerializer(el).data)
            elif hasattr(el, 'is_checked'):
                result.append(CheckboxElementSerializer(el).data)
            elif hasattr(el, 'content'):
                result.append(TextElementSerializer(el).data)
            elif hasattr(el, 'linked_page'):
                result.append(LinkElementSerializer(el).data)
        return result

    def get_icon(self, obj):
        if obj.icon and hasattr(obj.icon, 'url'):
            try:
                with obj.icon.open('rb') as f:
                    return base64.b64encode(f.read()).decode('utf-8')
            except Exception:
                return ''
        return ''

    def get_pages(self, obj):
        subpages = obj.subpages.all()
        return PageSerializer(subpages, many=True).data

    def create(self, validated_data):
        return super().create(validated_data)

    def update(self, instance, validated_data):
        return super().update(instance, validated_data)


class WorkspaceSerializer(serializers.ModelSerializer):
    pages = serializers.SerializerMethodField()
    elements = serializers.SerializerMethodField()
    icon = serializers.SerializerMethodField()
    created_at = serializers.SerializerMethodField()

    class Meta:
        model = Workspace
        fields = ['title', 'status', 'created_at', 'icon', 'elements', 'pages']
        read_only_fields = ['created_at']

    def get_icon(self, obj):
        if obj.icon and hasattr(obj.icon, 'url'):
            try:
                with obj.icon.open('rb') as f:
                    return base64.b64encode(f.read()).decode('utf-8')
            except Exception:
                return ''
        return ''

    def get_created_at(self, obj):
        return obj.created_at.isoformat()

    def get_pages(self, obj):
        subpages = obj.pages.filter(parent_page__isnull=True)
        return PageSerializer(subpages, many=True).data

    def get_elements(self, obj):
        # Собираем все элементы, у которых workspace=obj и page=None
        elements = []
        elements.extend(TextElement.objects.filter(workspace=obj, page=None))
        elements.extend(CheckboxElement.objects.filter(workspace=obj, page=None))
        elements.extend(ImageElement.objects.filter(workspace=obj, page=None))
        elements.extend(FileElement.objects.filter(workspace=obj, page=None))
        elements.extend(LinkElement.objects.filter(workspace=obj, page=None))
        # Сортируем по created_at, если есть поле created_at
        elements.sort(
            key=lambda el: getattr(el, 'created_at', None) or 0
        )
        result = []
        for el in elements:
            if isinstance(el, TextElement):
                result.append(TextElementSerializer(el).data)
            elif isinstance(el, CheckboxElement):
                result.append(CheckboxElementSerializer(el).data)
            elif isinstance(el, ImageElement):
                result.append(ImageElementSerializer(el).data)
            elif isinstance(el, FileElement):
                result.append(FileElementSerializer(el).data)
            elif isinstance(el, LinkElement):
                result.append(LinkElementSerializer(el).data)
        return result

    def create(self, validated_data):
        pages_data = self.initial_data.get('pages', [])
        elements_data = self.initial_data.get('elements', {})
        # Исправление: если elements_data — список, преобразуем в dict по типам
        if isinstance(elements_data, list):
            elements_dict = {
                'images': [], 'files': [], 'checkboxes': [], 'texts': [], 'links': []
            }
            for el in elements_data:
                el_type = el.get('type')
                if el_type == 'ImageItem':
                    elements_dict['images'].append(el)
                elif el_type == 'FileItem':
                    elements_dict['files'].append(el)
                elif el_type == 'CheckboxItem':
                    elements_dict['checkboxes'].append(el)
                elif el_type == 'TextItem':
                    elements_dict['texts'].append(el)
                elif el_type == 'SubspaceLinkItem':
                    elements_dict['links'].append(el)
            elements_data = elements_dict
        icon_b64 = self.initial_data.get('icon')
        title = validated_data['title']
        status = validated_data.get('status', 'not_started')

        print("[WorkspaceSerializer.create] title=", title, "status=", status)
        print("[WorkspaceSerializer.create] icon_b64 exists:", bool(icon_b64))
        print("[WorkspaceSerializer.create] elements_data:")
        print(pprint.pformat(elements_data))
        print("[WorkspaceSerializer.create] pages_data:")
        print(pprint.pformat(pages_data))

        # Исправление: если guest, не передавать author
        if self.context.get('guest'):
            workspace = Workspace.objects.create(
                title=title,
                status=status
            )
        else:
            workspace = Workspace.objects.create(
                title=title,
                status=status,
                author=self.context['request'].user
            )

        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                workspace.icon.save(
                    f'{title}_icon.png',
                    ContentFile(icon_data),
                    save=True
                )
                print(f"[WorkspaceSerializer.create] icon saved for {title}")
            except Exception as e:
                print(f"Error saving icon for workspace {title}: {e}")

        workspace.save()

        try:
            self._create_elements(workspace, elements_data)
            print(f"[WorkspaceSerializer.create] elements created for {title}")
        except Exception as e:
            print(f"Error creating elements for workspace {title}: {e}")
            print(pprint.pformat(elements_data))

        try:
            self._create_pages(workspace, pages_data)
            print(f"[WorkspaceSerializer.create] pages created for {title}")
        except Exception as e:
            print(f"Error creating pages for workspace {title}: {e}")
            print(pprint.pformat(pages_data))

        return workspace
    
    def _create_elements(self, obj, elements_data):
        # Исправление: если elements_data — список, преобразуем в dict по типам
        if isinstance(elements_data, list):
            elements_dict = {
                'images': [], 'files': [], 'checkboxes': [], 'texts': [], 'links': []
            }
            for el in elements_data:
                el_type = el.get('type')
                if el_type == 'ImageItem':
                    elements_dict['images'].append(el)
                elif el_type == 'FileItem':
                    elements_dict['files'].append(el)
                elif el_type == 'CheckboxItem':
                    elements_dict['checkboxes'].append(el)
                elif el_type == 'TextItem':
                    elements_dict['texts'].append(el)
                elif el_type == 'SubspaceLinkItem':
                    elements_dict['links'].append(el)
            elements_data = elements_dict
        """
        obj: Workspace или Page
        """
        for el_type, el_list in elements_data.items():
            for el in el_list:
                if el_type == 'images':
                    if isinstance(obj, Workspace):
                        ImageElement.objects.create(
                            workspace=obj, page=None, **el
                        )
                    else:
                        ImageElement.objects.create(
                            page=obj, workspace=None, **el
                        )
                elif el_type == 'files':
                    if isinstance(obj, Workspace):
                        FileElement.objects.create(
                            workspace=obj, page=None, **el
                        )
                    else:
                        FileElement.objects.create(
                            page=obj, workspace=None, **el
                        )
                elif el_type == 'checkboxes':
                    if isinstance(obj, Workspace):
                        CheckboxElement.objects.create(
                            workspace=obj, page=None, **el
                        )
                    else:
                        CheckboxElement.objects.create(
                            page=obj, workspace=None, **el
                        )
                elif el_type == 'texts':
                    if isinstance(obj, Workspace):
                        TextElement.objects.create(
                            workspace=obj, page=None, **el
                        )
                    else:
                        TextElement.objects.create(
                            page=obj, workspace=None, **el
                        )
                elif el_type == 'links':
                    linked_page_title = el.pop('linked_page', None)
                    if linked_page_title:
                        linked_page = Page.objects.filter(
                            space=obj.space if isinstance(obj, Page) else obj,
                            title=linked_page_title
                        ).first()
                        if linked_page:
                            if isinstance(obj, Workspace):
                                LinkElement.objects.create(
                                    workspace=obj, page=None,
                                    linked_page=linked_page, **el
                                )
                            else:
                                LinkElement.objects.create(
                                    page=obj, workspace=None,
                                    linked_page=linked_page, **el
                                )
                    else:
                        if isinstance(obj, Workspace):
                            LinkElement.objects.create(
                                workspace=obj, page=None, **el
                            )
                        else:
                            LinkElement.objects.create(
                                page=obj, workspace=None, **el
                            )

    def _create_pages(self, workspace, pages_data, parent_page=None):
        for page_data in pages_data:
            elements_data = page_data.pop('elements', {})
            subpages_data = page_data.pop('pages', [])
            icon_b64 = page_data.pop('icon', None)

            page = Page.objects.create(
                space=workspace,
                parent_page=parent_page,
                **page_data
            )

            if icon_b64:
                try:
                    icon_data = base64.b64decode(icon_b64)
                    page.icon.save(
                        f'{page.title}_icon.png',
                        ContentFile(icon_data),
                        save=True
                    )
                except Exception as e:
                    logger.error(
                        f"Error saving page icon: {e}"
                    )

            self._create_elements(page, elements_data)
            self._create_pages(workspace, subpages_data, page)

    def update(self, instance, validated_data):
        instance.status = validated_data.get('status', instance.status)
        instance.save()

        icon_b64 = self.initial_data.get('icon')
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                instance.icon.save(
                    f'{instance.title}_icon.png',
                    ContentFile(icon_data),
                    save=True
                )
            except Exception as e:
                logger.error(f"Error updating icon: {e}")

        elements_data = self.initial_data.get('elements', {})
        pages_data = self.initial_data.get('pages', [])

        # Удаляем существующие элементы
        for subclass in Element.__subclasses__():
            subclass.objects.filter(workspace=instance).delete()

        # Создаем новые элементы
        self._create_elements(instance, elements_data)

        # Удаляем существующие страницы
        instance.pages.all().delete()

        # Создаем новые страницы
        self._create_pages(instance, pages_data)

        return instance
    