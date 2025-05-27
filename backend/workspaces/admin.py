from django.contrib import admin
from django.utils.translation import gettext_lazy as _
from .models import (
    Workspace, Page, ImageElement, FileElement,
    CheckboxElement, TextElement, LinkElement
)


class BaseElementInline(admin.StackedInline):
    extra = 0
    readonly_fields = ['created_at']


class ImageElementInline(BaseElementInline):
    model = ImageElement
    fields = ['image']


class FileElementInline(BaseElementInline):
    model = FileElement
    fields = ['file']


class CheckboxElementInline(BaseElementInline):
    model = CheckboxElement
    fields = ['text', 'is_checked']


class TextElementInline(BaseElementInline):
    model = TextElement
    fields = ['content']


# Отдельные Inline классы для LinkElement
class PageLinkElementInline(BaseElementInline):
    model = LinkElement
    fields = ['linked_page', 'created_at']
    fk_name = 'page'  # Связь через поле 'page'

class WorkspaceLinkElementInline(BaseElementInline):
    model = LinkElement
    fields = ['linked_page', 'created_at']
    fk_name = 'workspace'  # Связь через поле 'workspace'

@admin.register(Workspace)
class WorkspaceAdmin(admin.ModelAdmin):
    list_display = ['title', 'author', 'status', 'created_at']
    list_filter = ['status', 'created_at']
    search_fields = ['title', 'author__username']
    readonly_fields = ['created_at']
    inlines = [
        ImageElementInline,
        FileElementInline,
        CheckboxElementInline,
        TextElementInline,
        WorkspaceLinkElementInline  # Используем версию для рабочих пространств
    ]

    fieldsets = (
        (None, {
            'fields': ('title', 'author', 'status', 'created_at')
        }),
        (_('Медиа'), {
            'fields': ('icon', 'banner'),
            'classes': ('collapse',)
        }),
        (_('Дополнительная информация'), {
            'fields': ('tags', 'info', 'start_date', 'end_date'),
            'classes': ('collapse',)
        }),
    )


@admin.register(Page)
class PageAdmin(admin.ModelAdmin):
    list_display = ['title', 'space', 'get_full_path', 'created_at']
    list_filter = ['space', 'created_at']
    search_fields = ['title', 'space__title']
    readonly_fields = ['created_at']
    inlines = [
        ImageElementInline,
        FileElementInline,
        CheckboxElementInline,
        TextElementInline,
        PageLinkElementInline  # Используем версию для страниц
    ]

    fieldsets = (
        (None, {
            'fields': ('title', 'space', 'parent_page', 'icon', 'created_at')
        }),
    )

    def get_full_path(self, obj):
        return obj.get_full_path()
    get_full_path.short_description = _('Полный путь')


@admin.register(ImageElement)
class ImageElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'get_related', 'created_at']
    list_filter = ['created_at']
    readonly_fields = ['created_at']

    def get_related(self, obj):
        return obj.workspace.title if obj.workspace else obj.page.title
    get_related.short_description = _('Пространство/Страница')


@admin.register(FileElement)
class FileElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'get_related', 'created_at']
    list_filter = ['created_at']
    readonly_fields = ['created_at']

    def get_related(self, obj):
        return obj.workspace.title if obj.workspace else obj.page.title
    get_related.short_description = _('Пространство/Страница')


@admin.register(CheckboxElement)
class CheckboxElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'get_related', 'text', 'is_checked', 'created_at']
    list_filter = ['created_at']
    readonly_fields = ['created_at']

    def get_related(self, obj):
        return obj.workspace.title if obj.workspace else obj.page.title
    get_related.short_description = _('Пространство/Страница')


@admin.register(TextElement)
class TextElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'get_related', 'content_short', 'created_at']
    list_filter = ['created_at']
    readonly_fields = ['created_at']

    def get_related(self, obj):
        return obj.workspace.title if obj.workspace else obj.page.title
    get_related.short_description = _('Пространство/Страница')

    def content_short(self, obj):
        return obj.content[:50] + '...' if len(obj.content) > 50 else obj.content
    content_short.short_description = _('Содержимое')


@admin.register(LinkElement)
class LinkElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'get_related', 'linked_page', 'created_at']
    list_filter = ['created_at']
    readonly_fields = ['created_at']

    def get_related(self, obj):
        return obj.workspace.title if obj.workspace else obj.page.title
    get_related.short_description = _('Пространство/Страница')
