import base64
from django.core.files.base import ContentFile
from rest_framework import serializers
from workspaces.models import (
    Workspace, Page, Element, ImageElement,
    FileElement, CheckboxElement,
    TextElement, LinkElement
)
import logging

logger = logging.getLogger(__name__)

class Base64ImageField(serializers.ImageField):
    def to_internal_value(self, data):
        if isinstance(data, str) and data.startswith("data:image"):
            format, imgstr = data.split(";base64,")
            ext = format.split("/")[-1]
            data = ContentFile(base64.b64decode(imgstr), name="temp." + ext)
        return super().to_internal_value(data)


class ImageElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = ImageElement
        fields = ['image', 'created_at']
        read_only_fields = ['created_at']


class FileElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = FileElement
        fields = ['file', 'created_at']
        read_only_fields = ['created_at']


class CheckboxElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = CheckboxElement
        fields = ['text', 'is_checked', 'created_at']
        read_only_fields = ['created_at']


class TextElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = TextElement
        fields = ['content', 'created_at']
        read_only_fields = ['created_at']


class LinkElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = LinkElement
        fields = ['linked_page', 'created_at']
        read_only_fields = ['created_at']


class PageSerializer(serializers.ModelSerializer):
    elements = serializers.SerializerMethodField()
    icon = serializers.SerializerMethodField()
    pages = serializers.SerializerMethodField()

    class Meta:
        model = Page
        fields = [
            'title', 'elements', 'created_at', 'icon', 'pages'
        ]
        read_only_fields = ['created_at']

    def get_elements(self, obj):
        elements = []
        elements.extend(ImageElementSerializer(
            obj.imageelement_elements.all(), many=True
        ).data)
        elements.extend(FileElementSerializer(
            obj.fileelement_elements.all(), many=True
        ).data)
        elements.extend(CheckboxElementSerializer(
            obj.checkboxelement_elements.all(), many=True
        ).data)
        elements.extend(TextElementSerializer(
            obj.textelement_elements.all(), many=True
        ).data)
        elements.extend(LinkElementSerializer(
            obj.linkelement_elements.all(), many=True
        ).data)
        return elements

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
        elements = []
        for subclass in Element.__subclasses__():
            elements.extend(subclass.objects.filter(workspace=obj))
        return {
            'images': ImageElementSerializer(obj.imageelement_elements.all(), many=True).data,
            'files': FileElementSerializer(obj.fileelement_elements.all(), many=True).data,
            'checkboxes': CheckboxElementSerializer(obj.checkboxelement_elements.all(), many=True).data,
            'texts': TextElementSerializer(obj.textelement_elements.all(), many=True).data,
            'links': LinkElementSerializer(obj.linkelement_elements.all(), many=True).data,
        }

    def create(self, validated_data):
        pages_data = self.initial_data.get('pages', [])
        elements_data = self.initial_data.get('elements', {})
        icon_b64 = self.initial_data.get('icon')
        title = validated_data['title']
        status = validated_data.get('status', 'not_started')

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
            except Exception as e:
                logger.error(f"Error saving icon: {e}")

        workspace.save()

        self._create_elements(workspace, elements_data)
        self._create_pages(workspace, pages_data)
        return workspace
    
    def _create_elements(self, workspace, elements_data):
        for el_type, el_list in elements_data.items():
            for el in el_list:
                if el_type == 'images':
                    ImageElement.objects.create(workspace=workspace, **el)
                elif el_type == 'files':
                    FileElement.objects.create(workspace=workspace, **el)
                elif el_type == 'checkboxes':
                    CheckboxElement.objects.create(workspace=workspace, **el)
                elif el_type == 'texts':
                    TextElement.objects.create(workspace=workspace, **el)
                elif el_type == 'links':
                    linked_page_title = el.pop('linked_page', None)
                    if linked_page_title:
                        linked_page = Page.objects.filter(space=workspace, title=linked_page_title).first()
                        if linked_page:
                            LinkElement.objects.create(workspace=workspace, linked_page=linked_page, **el)
                    else:
                        LinkElement.objects.create(workspace=workspace, **el)

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
                    logger.error(f"Error saving page icon: {e}")

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
    