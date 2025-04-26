from django.contrib import admin
from django.db import models
from .models import Workspace, Page, ContentElement, TextElement, ImageElement, FileElement, LinkElement
from django.contrib.contenttypes.models import ContentType

# Инлайн-классы для различных типов элементов
class TextElementInline(admin.StackedInline):
    model = TextElement
    extra = 0
    fields = ('content', 'order')

class ImageElementInline(admin.StackedInline):
    model = ImageElement
    extra = 0
    fields = ('image', 'order')

class FileElementInline(admin.StackedInline):
    model = FileElement
    extra = 0
    fields = ('file', 'order')

class LinkElementInline(admin.StackedInline):
    model = LinkElement
    extra = 0
    fields = ('linked_page', 'order')
    fk_name = 'page'

# Админка для модели Page
class PageAdmin(admin.ModelAdmin):
    list_display = ('name', 'workspace', 'created_at', 'updated_at')
    list_filter = ('workspace',)
    search_fields = ('name', 'workspace__name')
    prepopulated_fields = {'slug': ('name',)}  # Автоматическое заполнение slug на основе имени

    # Добавляем инлайны для всех типов элементов
    inlines = [
        TextElementInline,
        ImageElementInline,
        FileElementInline,
        LinkElementInline,
    ]

    def save_formset(self, request, form, formset, change):
        """
        Переопределяем метод для автоматической нумерации элементов.
        """
        instances = formset.save(commit=False)
        for instance in instances:
            if not instance.order:
                max_order = ContentElement.objects.filter(page=instance.page).aggregate(models.Max('order'))['order__max']
                instance.order = (max_order or 0) + 1
            # Создаем связанный объект ContentElement
            if not hasattr(instance, 'contentelement'):
                content_type = ContentType.objects.get_for_model(instance.__class__)
                instance.contentelement = ContentElement.objects.create(
                    page=instance.page,
                    content_type=content_type,
                    object_id=instance.id
                )
            instance.save()
        formset.save_m2m()

# Регистрация моделей в админке
admin.site.register(Workspace)
admin.site.register(Page, PageAdmin)

# Отдельные админки для подклассов
@admin.register(TextElement)
class TextElementAdmin(admin.ModelAdmin):
    list_display = ('id', 'page', 'order', 'content')
    list_filter = ('page',)
    search_fields = ('content',)

    def save_model(self, request, obj, form, change):
        # Автоматически создаем связанный объект ContentElement
        if not hasattr(obj, 'contentelement'):
            content_type = ContentType.objects.get_for_model(obj.__class__)
            obj.contentelement = ContentElement.objects.create(
                page=obj.page,
                content_type=content_type,
                object_id=obj.id
            )
        super().save_model(request, obj, form, change)

# Аналогично для других подклассов
@admin.register(ImageElement)
class ImageElementAdmin(admin.ModelAdmin):
    list_display = ('id', 'page', 'order', 'image')
    list_filter = ('page',)

    def save_model(self, request, obj, form, change):
        if not hasattr(obj, 'contentelement'):
            content_type = ContentType.objects.get_for_model(obj.__class__)
            obj.contentelement = ContentElement.objects.create(
                page=obj.page,
                content_type=content_type,
                object_id=obj.id
            )
        super().save_model(request, obj, form, change)

@admin.register(FileElement)
class FileElementAdmin(admin.ModelAdmin):
    list_display = ('id', 'page', 'order', 'file')
    list_filter = ('page',)

    def save_model(self, request, obj, form, change):
        if not hasattr(obj, 'contentelement'):
            content_type = ContentType.objects.get_for_model(obj.__class__)
            obj.contentelement = ContentElement.objects.create(
                page=obj.page,
                content_type=content_type,
                object_id=obj.id
            )
        super().save_model(request, obj, form, change)

@admin.register(LinkElement)
class LinkElementAdmin(admin.ModelAdmin):
    list_display = ('id', 'page', 'order', 'linked_page')
    list_filter = ('page',)

    def save_model(self, request, obj, form, change):
        if not hasattr(obj, 'contentelement'):
            content_type = ContentType.objects.get_for_model(obj.__class__)
            obj.contentelement = ContentElement.objects.create(
                page=obj.page,
                content_type=content_type,
                object_id=obj.id
            )
        super().save_model(request, obj, form, change)