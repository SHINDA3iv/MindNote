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
        for element in obj.elements.all():
            element_data = {
                'id': element.id,
                'element_type': element.element_type,
                'created_at': element.created_at
            }
            
            if element.element_type == 'image':
                element_data['image'] = element.imageelement.image.url if element.imageelement.image else None
            elif element.element_type == 'file':
                element_data['file'] = element.fileelement.file.url if element.fileelement.file else None
            elif element.element_type == 'checkbox':
                element_data['text'] = element.checkboxelement.text
                element_data['is_checked'] = element.checkboxelement.is_checked
            elif element.element_type == 'text':
                element_data['content'] = element.textelement.content
            elif element.element_type == 'link':
                element_data['linked_page'] = {
                    'id': element.linkelement.linked_page.id,
                    'title': element.linkelement.linked_page.title,
                    'link': element.linkelement.linked_page.link
                }
            
            elements.append(element_data)
        return elements

    def validate(self, data):
        if self.instance and self.instance.is_main:
            raise serializers.ValidationError("Нельзя изменить главную страницу.")
        return data


class ElementSerializer(serializers.ModelSerializer):
    class Meta:
        model = Element
        fields = [
            'id', 'element_type', 'created_at'
        ]
        read_only_fields = ['id', 'created_at']

    def create(self, validated_data):
        page = self.context.get('page')
        if not page:
            raise serializers.ValidationError("Страница не указана.")
        
        element = Element.objects.create(page=page, **validated_data)
        
        element_data = self.context.get('element_data', {})
        
        if element.element_type == 'image' and 'image' in element_data:
            ImageElement.objects.create(element=element, image=element_data['image'])
        elif element.element_type == 'file' and 'file' in element_data:
            FileElement.objects.create(element=element, file=element_data['file'])
        elif element.element_type == 'checkbox':
            CheckboxElement.objects.create(
                element=element,
                text=element_data.get('text', ''),
                is_checked=element_data.get('is_checked', False)
            )
        elif element.element_type == 'text' and 'content' in element_data:
            TextElement.objects.create(element=element, content=element_data['content'])
        elif element.element_type == 'link' and 'linked_page' in element_data:
            LinkElement.objects.create(
                element=element,
                linked_page_id=element_data['linked_page']
            )
        
        return element

    def update(self, instance, validated_data):
        element_data = self.context.get('element_data', {})
        
        if instance.element_type == 'image' and 'image' in element_data:
            if hasattr(instance, 'imageelement'):
                instance.imageelement.image = element_data['image']
                instance.imageelement.save()
        elif instance.element_type == 'file' and 'file' in element_data:
            if hasattr(instance, 'fileelement'):
                instance.fileelement.file = element_data['file']
                instance.fileelement.save()
        elif instance.element_type == 'checkbox':
            if hasattr(instance, 'checkboxelement'):
                instance.checkboxelement.text = element_data.get('text', instance.checkboxelement.text)
                instance.checkboxelement.is_checked = element_data.get('is_checked', instance.checkboxelement.is_checked)
                instance.checkboxelement.save()
        elif instance.element_type == 'text' and 'content' in element_data:
            if hasattr(instance, 'textelement'):
                instance.textelement.content = element_data['content']
                instance.textelement.save()
        elif instance.element_type == 'link' and 'linked_page' in element_data:
            if hasattr(instance, 'linkelement'):
                instance.linkelement.linked_page_id = element_data['linked_page']
                instance.linkelement.save()
        
        return instance
    