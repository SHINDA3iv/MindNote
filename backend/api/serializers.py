import base64
from django.core.files.base import ContentFile
from rest_framework import serializers
from workspaces.models import *

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

    class Meta:
        model = Page
        fields = ['title', 'is_main', 'elements', 'created_at', 'icon']
        read_only_fields = ['created_at']

    def get_elements(self, obj):
        elements = []
        elements.extend(ImageElementSerializer(obj.imageelement_elements.all(), many=True).data)
        elements.extend(FileElementSerializer(obj.fileelement_elements.all(), many=True).data)
        elements.extend(CheckboxElementSerializer(obj.checkboxelement_elements.all(), many=True).data)
        elements.extend(TextElementSerializer(obj.textelement_elements.all(), many=True).data)
        elements.extend(LinkElementSerializer(obj.linkelement_elements.all(), many=True).data)
        return elements

    def get_icon(self, obj):
        # Если это главная страница, вернуть иконку родительского Workspace
        if obj.is_main and obj.space and obj.space.icon and hasattr(obj.space.icon, 'url'):
            try:
                with obj.space.icon.open('rb') as f:
                    return base64.b64encode(f.read()).decode('utf-8')
            except Exception:
                return ''
        # Иначе вернуть свою иконку
        if obj.icon and hasattr(obj.icon, 'url'):
            try:
                with obj.icon.open('rb') as f:
                    return base64.b64encode(f.read()).decode('utf-8')
            except Exception:
                return ''
        return ''

    def validate(self, data):
        if self.instance and self.instance.is_main:
            raise serializers.ValidationError("Нельзя изменить главную страницу.")
        return data

    def update(self, instance, validated_data):
        icon_b64 = self.initial_data.get('icon')
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                instance.icon.save(f'{instance.title}_icon.png', ContentFile(icon_data), save=False)
            except Exception:
                pass
        return super().update(instance, validated_data)

    def create(self, validated_data):
        icon_b64 = self.initial_data.get('icon')
        instance = super().create(validated_data)
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                instance.icon.save(f'{instance.title}_icon.png', ContentFile(icon_data), save=True)
            except Exception:
                pass
        return instance


class WorkspaceSerializer(serializers.ModelSerializer):
    pages = PageSerializer(many=True, read_only=True)
    icon = serializers.SerializerMethodField()

    class Meta:
        model = Workspace
        fields = ['title', 'status', 'pages', 'created_at', 'icon']
        read_only_fields = ['created_at']

    def get_icon(self, obj):
        if obj.icon and hasattr(obj.icon, 'url'):
            try:
                with obj.icon.open('rb') as f:
                    return base64.b64encode(f.read()).decode('utf-8')
            except Exception:
                return ''
        return ''

    def update(self, instance, validated_data):
        icon_b64 = self.initial_data.get('icon')
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                instance.icon.save(f'{instance.title}_icon.png', ContentFile(icon_data), save=False)
            except Exception:
                pass
        return super().update(instance, validated_data)

    def create(self, validated_data):
        icon_b64 = self.initial_data.get('icon')
        instance = super().create(validated_data)
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                instance.icon.save(f'{instance.title}_icon.png', ContentFile(icon_data), save=True)
            except Exception:
                pass
        return instance
    