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


class ContentElementSerializer(serializers.Serializer):
    id = serializers.IntegerField(read_only=True)
    type = serializers.SerializerMethodField()
    order = serializers.IntegerField()

    def get_type(self, obj):
        if obj.content_type.model == 'textelement':
            return 'text'
        elif obj.content_type.model == 'imageelement':
            return 'image'
        elif obj.content_type.model == 'fileelement':
            return 'file'
        elif obj.content_type.model == 'linkelement':
            return 'link'
        return None


class PageSerializer(serializers.ModelSerializer):
    icon = Base64ImageField(required=False, allow_null=True)
    banner = Base64ImageField(required=False, allow_null=True)
    elements = serializers.SerializerMethodField()

    class Meta:
        model = Page
        fields = ("id", "name", "slug", "icon", "banner", "created_at", "elements")

    def get_elements(self, obj):
        elements = obj.page_elements.all().order_by('order')
        return ContentElementPolymorphicSerializer(elements, many=True, context=self.context).data


class WorkspaceSerializer(serializers.ModelSerializer):
    first_page = SerializerMethodField()

    class Meta:
        model = Workspace
        fields = ("name", "first_page")

    def get_first_page(self, obj):
        first_page = obj.pages.first()
        if first_page:
            return PageSerializer(first_page).data
        return None


class TextElementSerializer(ContentElementSerializer):
    content = serializers.CharField(source='content_object.content')

class ImageElementSerializer(ContentElementSerializer):
    image = Base64ImageField(source='content_object.image')

class FileElementSerializer(ContentElementSerializer):
    file = serializers.FileField(source='content_object.file')

class LinkElementSerializer(ContentElementSerializer):
    url = serializers.URLField(source='content_object.url')
    linked_page = serializers.PrimaryKeyRelatedField(
        queryset=Page.objects.all(), allow_null=True, source='content_object.linked_page'
    )


class ContentElementPolymorphicSerializer(serializers.Serializer):
    def to_representation(self, instance):
        content_object = instance.content_object

        if isinstance(content_object, TextElement):
            return TextElementSerializer(content_object).data
        elif isinstance(content_object, ImageElement):
            return ImageElementSerializer(content_object).data
        elif isinstance(content_object, FileElement):
            return FileElementSerializer(content_object).data
        elif isinstance(content_object, LinkElement):
            return LinkElementSerializer(content_object).data
        return super().to_representation(instance)