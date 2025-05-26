import base64
from django.core.files.base import ContentFile
from rest_framework import serializers
from workspaces.models import (
    Workspace, Page, ImageElement,
    FileElement, CheckboxElement,
    TextElement, LinkElement
)


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
            'title', 'is_main', 'elements', 'created_at', 'icon', 'pages'
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
        # Только подстраницы (parent=this page)
        subpages = obj.subpages.all()
        return PageSerializer(subpages, many=True).data

    def create(self, validated_data):
        # Не используется напрямую, вложенные страницы создаются вручную
        return super().create(validated_data)

    def update(self, instance, validated_data):
        # Не используется напрямую
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

    def get_main_page(self, obj):
        # Получить главную страницу (is_main=True)
        return obj.pages.filter(is_main=True).first()

    def get_elements(self, obj):
        main_page = self.get_main_page(obj)
        if main_page:
            return PageSerializer(main_page).get_elements(main_page)
        return []

    def get_icon(self, obj):
        main_page = self.get_main_page(obj)
        if main_page:
            return PageSerializer(main_page).get_icon(main_page)
        return ''

    def get_created_at(self, obj):
        main_page = self.get_main_page(obj)
        if main_page:
            return main_page.created_at.isoformat()
        return ''

    def get_pages(self, obj):
        main_page = self.get_main_page(obj)
        if not main_page:
            return []
        # Только подстраницы главной страницы (parent=main_page, is_main=False)
        subpages = main_page.subpages.all()
        return PageSerializer(subpages, many=True).data

    def to_representation(self, instance):
        # Обычная сериализация
        return super().to_representation(instance)

    def create(self, validated_data):
        # Создать workspace и главную страницу, а также вложенные страницы
        pages_data = self.initial_data.get('pages', [])
        elements = self.initial_data.get('elements', [])
        icon_b64 = self.initial_data.get('icon')
        created_at = self.initial_data.get('created_at')
        title = validated_data['title']
        status = validated_data.get('status', 'not_started')
        workspace = Workspace.objects.create(
            title=title,
            status=status,
            author=self.context['request'].user
        )
        # Главная страница
        main_page = Page.objects.create(
            space=workspace,
            title=title,
            is_main=True
        )
        if created_at:
            main_page.created_at = created_at
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                main_page.icon.save(
                    f'{title}_icon.png',
                    ContentFile(icon_data),
                    save=True
                )
            except Exception:
                pass
        main_page.save()
        # Элементы главной страницы
        for el in elements:
            el_type = el.get('type')
            if el_type == 'TextItem':
                TextElement.objects.create(
                    page=main_page,
                    content=el.get('content', '')
                )
            elif el_type == 'CheckboxItem':
                CheckboxElement.objects.create(
                    page=main_page,
                    text=el.get('label', ''),
                    is_checked=el.get('checked', False)
                )
            elif el_type == 'FileItem':
                FileElement.objects.create(
                    page=main_page,
                    file=el.get('filePath', '')
                )
            elif el_type == 'SubspaceLinkItem':
                # Ссылка на подпространство: ищем страницу по title среди уже созданных
                subspace_title = el.get('subspaceTitle', '')
                linked_page = Page.objects.filter(space=workspace, title=subspace_title).first()
                if linked_page:
                    LinkElement.objects.create(
                        page=main_page,
                        linked_page=linked_page
                    )
        # Вложенные страницы рекурсивно
        self._create_subpages(main_page, pages_data)
        return workspace

    def _create_subpages(self, parent_page, pages_data):
        for page_data in pages_data:
            # Только подстраницы (is_main=False)
            subpage = Page.objects.create(
                space=parent_page.space,
                title=page_data.get('title'),
                is_main=False,
                parent=parent_page
            )
            icon_b64 = page_data.get('icon')
            if icon_b64:
                try:
                    icon_data = base64.b64decode(icon_b64)
                    subpage.icon.save(
                        f'{subpage.title}_icon.png',
                        ContentFile(icon_data),
                        save=True
                    )
                except Exception:
                    pass
            subpage.save()
            # Элементы
            for el in page_data.get('elements', []):
                el_type = el.get('type')
                if el_type == 'TextItem':
                    TextElement.objects.create(
                        page=subpage,
                        content=el.get('content', '')
                    )
                elif el_type == 'CheckboxItem':
                    CheckboxElement.objects.create(
                        page=subpage,
                        text=el.get('label', ''),
                        is_checked=el.get('checked', False)
                    )
                elif el_type == 'FileItem':
                    FileElement.objects.create(
                        page=subpage,
                        file=el.get('filePath', '')
                    )
                elif el_type == 'SubspaceLinkItem':
                    subspace_title = el.get('subspaceTitle', '')
                    linked_page = Page.objects.filter(
                        space=subpage.space, title=subspace_title
                    ).first()
                    if linked_page:
                        LinkElement.objects.create(
                            page=subpage,
                            linked_page=linked_page
                        )
            # Рекурсивно вложенные страницы
            self._create_subpages(subpage, page_data.get('pages', []))

    def update(self, instance, validated_data):
        # Аналогично create, но с удалением старых страниц/элементов
        main_page = instance.pages.filter(is_main=True).first()
        if not main_page:
            main_page = Page.objects.create(
                space=instance,
                title=instance.title,
                is_main=True
            )
        # Обновить поля
        instance.status = validated_data.get('status', instance.status)
        instance.save()
        # Обновить главную страницу
        main_page.title = instance.title
        main_page.save()
        # Обновить иконку
        icon_b64 = self.initial_data.get('icon')
        if icon_b64:
            try:
                icon_data = base64.b64decode(icon_b64)
                main_page.icon.save(
                    f'{main_page.title}_icon.png',
                    ContentFile(icon_data),
                    save=True
                )
            except Exception:
                pass
        # Удалить старые элементы и страницы
        main_page.imageelement_elements.all().delete()
        main_page.fileelement_elements.all().delete()
        main_page.checkboxelement_elements.all().delete()
        main_page.textelement_elements.all().delete()
        main_page.linkelement_elements.all().delete()
        main_page.subpages.all().delete()
        # Добавить новые элементы
        for el in self.initial_data.get('elements', []):
            el_type = el.get('type')
            if el_type == 'TextItem':
                TextElement.objects.create(
                    page=main_page,
                    content=el.get('content', '')
                )
            elif el_type == 'CheckboxItem':
                CheckboxElement.objects.create(
                    page=main_page,
                    text=el.get('label', ''),
                    is_checked=el.get('checked', False)
                )
            elif el_type == 'FileItem':
                FileElement.objects.create(
                    page=main_page,
                    file=el.get('filePath', '')
                )
            elif el_type == 'SubspaceLinkItem':
                subspace_title = el.get('subspaceTitle', '')
                linked_page = Page.objects.filter(
                    space=main_page.space, title=subspace_title
                ).first()
                if linked_page:
                    LinkElement.objects.create(
                        page=main_page,
                        linked_page=linked_page
                    )
            # ... другие типы ...
        # Вложенные страницы рекурсивно
        self._create_subpages(main_page, self.initial_data.get('pages', []))
        return instance
    