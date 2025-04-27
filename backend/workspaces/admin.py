from django.contrib import admin
from django.utils.translation import gettext_lazy as _
from .models import (
    Workspace, Page, ImageElement, FileElement,
    CheckboxElement, TextElement, LinkElement
)


class ImageElementInline(admin.StackedInline):
    model = ImageElement
    extra = 0
    fields = ['image']


class FileElementInline(admin.StackedInline):
    model = FileElement
    extra = 0
    fields = ['file']


class CheckboxElementInline(admin.StackedInline):
    model = CheckboxElement
    extra = 0
    fields = ['text', 'is_checked']


class TextElementInline(admin.StackedInline):
    model = TextElement
    extra = 0
    fields = ['content']


class LinkElementInline(admin.StackedInline):
    model = LinkElement
    extra = 0
    fields = ['linked_page']
    fk_name = 'page'


class PageInline(admin.StackedInline):
    model = Page
    extra = 0
    fields = ['title', 'link', 'is_main']
    readonly_fields = ['link']
    inlines = [
        ImageElementInline,
        FileElementInline,
        CheckboxElementInline,
        TextElementInline,
        LinkElementInline
    ]


@admin.register(Workspace)
class WorkspaceAdmin(admin.ModelAdmin):
    list_display = ['title', 'author', 'created_at', 'status']
    list_filter = ['status', 'created_at']
    search_fields = ['title', 'author__username']
    readonly_fields = ['created_at']
    inlines = [PageInline]
    fieldsets = (
        (None, {
            'fields': ('title', 'author', 'created_at')
        }),
        (_('Дополнительная информация'), {
            'fields': ('icon', 'banner', 'tags', 'info'),
            'classes': ('collapse',)
        }),
        (_('Статус и даты'), {
            'fields': ('status', 'start_date', 'end_date'),
            'classes': ('collapse',)
        }),
    )


@admin.register(Page)
class PageAdmin(admin.ModelAdmin):
    list_display = ['title', 'space', 'link', 'is_main', 'created_at']
    list_filter = ['is_main', 'created_at']
    search_fields = ['title', 'space__title']
    readonly_fields = ['link', 'created_at']
    inlines = [
        ImageElementInline,
        FileElementInline,
        CheckboxElementInline,
        TextElementInline,
        LinkElementInline
    ]
    fieldsets = (
        (None, {
            'fields': ('space', 'title', 'link', 'is_main', 'created_at')
        }),
    )


@admin.register(ImageElement)
class ImageElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'page', 'created_at']
    list_filter = ['created_at']
    search_fields = ['page__title']
    readonly_fields = ['created_at']
    fieldsets = (
        (None, {
            'fields': ('page', 'image', 'created_at')
        }),
    )


@admin.register(FileElement)
class FileElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'page', 'created_at']
    list_filter = ['created_at']
    search_fields = ['page__title']
    readonly_fields = ['created_at']
    fieldsets = (
        (None, {
            'fields': ('page', 'file', 'created_at')
        }),
    )


@admin.register(CheckboxElement)
class CheckboxElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'page', 'text', 'is_checked', 'created_at']
    list_filter = ['is_checked', 'created_at']
    search_fields = ['page__title', 'text']
    readonly_fields = ['created_at']
    fieldsets = (
        (None, {
            'fields': ('page', 'text', 'is_checked', 'created_at')
        }),
    )


@admin.register(TextElement)
class TextElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'page', 'created_at']
    list_filter = ['created_at']
    search_fields = ['page__title', 'content']
    readonly_fields = ['created_at']
    fieldsets = (
        (None, {
            'fields': ('page', 'content', 'created_at')
        }),
    )


@admin.register(LinkElement)
class LinkElementAdmin(admin.ModelAdmin):
    list_display = ['id', 'page', 'linked_page', 'created_at']
    list_filter = ['created_at']
    search_fields = ['page__title', 'linked_page__title']
    readonly_fields = ['created_at']
    fieldsets = (
        (None, {
            'fields': ('page', 'linked_page', 'created_at')
        }),
    )

