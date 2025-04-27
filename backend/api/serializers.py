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


class WorkspaceSerializer(serializers.ModelSerializer):
    class Meta:
        model = Workspace
        fields = [
            'id', 'title', 'author', 'created_at', 'icon', 'banner',
            'tags', 'info', 'start_date', 'end_date', 'status'
        ]
        read_only_fields = ['id', 'author', 'created_at']

    def create(self, validated_data):
        validated_data['author'] = self.context['request'].user
        return super().create(validated_data)


class PageSerializer(serializers.ModelSerializer):
    elements = SerializerMethodField()

    class Meta:
        model = Page
        fields = ['id', 'space', 'title', 'link', 'is_main', 'created_at', 'elements']
        read_only_fields = ['id', 'created_at', 'link']

    def get_elements(self, obj):
        elements = []
        
        # Получаем все элементы разных типов
        image_elements = obj.image_elements.all()
        file_elements = obj.file_elements.all()
        checkbox_elements = obj.checkbox_elements.all()
        text_elements = obj.text_elements.all()
        link_elements = obj.link_elements.all()
        
        # Добавляем элементы изображений
        for element in image_elements:
            elements.append({
                'id': element.id,
                'element_type': 'image',
                'image': element.image.url if element.image else None,
                'created_at': element.created_at
            })
        
        # Добавляем элементы файлов
        for element in file_elements:
            elements.append({
                'id': element.id,
                'element_type': 'file',
                'file': element.file.url if element.file else None,
                'created_at': element.created_at
            })
        
        # Добавляем элементы чекбоксов
        for element in checkbox_elements:
            elements.append({
                'id': element.id,
                'element_type': 'checkbox',
                'text': element.text,
                'is_checked': element.is_checked,
                'created_at': element.created_at
            })
        
        # Добавляем элементы текстовых полей
        for element in text_elements:
            elements.append({
                'id': element.id,
                'element_type': 'text',
                'content': element.content,
                'created_at': element.created_at
            })
        
        # Добавляем элементы ссылок
        for element in link_elements:
            elements.append({
                'id': element.id,
                'element_type': 'link',
                'linked_page': {
                    'id': element.linked_page.id,
                    'title': element.linked_page.title,
                    'link': element.linked_page.link
                },
                'created_at': element.created_at
            })
        
        return elements

    def validate(self, data):
        if self.instance and self.instance.is_main:
            raise serializers.ValidationError("Нельзя изменить главную страницу.")
        return data


class ImageElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = ImageElement
        fields = ['id', 'image', 'created_at']
        read_only_fields = ['id', 'created_at']


class FileElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = FileElement
        fields = ['id', 'file', 'created_at']
        read_only_fields = ['id', 'created_at']


class CheckboxElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = CheckboxElement
        fields = ['id', 'text', 'is_checked', 'created_at']
        read_only_fields = ['id', 'created_at']


class TextElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = TextElement
        fields = ['id', 'content', 'created_at']
        read_only_fields = ['id', 'created_at']


class LinkElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = LinkElement
        fields = ['id', 'linked_page', 'created_at']
        read_only_fields = ['id', 'created_at']
    