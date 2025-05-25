import base64
from django.core.files.base import ContentFile
from rest_framework import serializers
from rest_framework.fields import SerializerMethodField
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

    class Meta:
        model = Page
        fields = ['title', 'is_main', 'elements', 'created_at']
        read_only_fields = ['created_at']

    def get_elements(self, obj):
        elements = []
        elements.extend(ImageElementSerializer(obj.imageelement_elements.all(), many=True).data)
        elements.extend(FileElementSerializer(obj.fileelement_elements.all(), many=True).data)
        elements.extend(CheckboxElementSerializer(obj.checkboxelement_elements.all(), many=True).data)
        elements.extend(TextElementSerializer(obj.textelement_elements.all(), many=True).data)
        elements.extend(LinkElementSerializer(obj.linkelement_elements.all(), many=True).data)
        return elements

    def validate(self, data):
        if self.instance and self.instance.is_main:
            raise serializers.ValidationError("Нельзя изменить главную страницу.")
        return data


class WorkspaceSerializer(serializers.ModelSerializer):
    pages = PageSerializer(many=True, read_only=True)

    class Meta:
        model = Workspace
        fields = ['title', 'status', 'pages', 'created_at']
        read_only_fields = ['created_at']
    