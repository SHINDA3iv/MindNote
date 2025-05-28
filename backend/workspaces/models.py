from django.db import models
from django.utils.translation import gettext_lazy as _
from django.core.exceptions import ValidationError
from django.conf import settings
from django.db.models.signals import post_save
from django.dispatch import receiver


class Workspace(models.Model):
    """
    Модель для рабочих пространств.
    """
    STATUS_CHOICES = [
        ('not_started', _('Не начато')),
        ('in_progress', _('В процессе')),
        ('completed', _('Завершено')),
    ]

    title = models.CharField(
        max_length=255,
        primary_key=True,
        verbose_name=_("Название")
    )
    author = models.ForeignKey(
        settings.AUTH_USER_MODEL,
        on_delete=models.CASCADE,
        related_name='spaces',
        verbose_name=_("Автор"),
        null=True,
        blank=True
    )
    created_at = models.DateTimeField(
        auto_now_add=True,
        verbose_name=_("Дата создания")
    )
    icon = models.ImageField(
        upload_to='space_icons/',
        blank=True,
        null=True,
        verbose_name=_("Иконка")
    )
    banner = models.ImageField(
        upload_to='space_banners/',
        blank=True,
        null=True,
        verbose_name=_("Баннер")
    )
    tags = models.CharField(
        max_length=255,
        blank=True,
        null=True,
        verbose_name=_("Теги")
    )
    info = models.TextField(
        blank=True,
        null=True,
        verbose_name=_("Информация")
    )
    start_date = models.DateField(
        blank=True,
        null=True,
        verbose_name=_("Дата начала")
    )
    end_date = models.DateField(
        blank=True,
        null=True,
        verbose_name=_("Дата окончания")
    )
    status = models.CharField(
        max_length=20,
        choices=STATUS_CHOICES,
        default='not_started',
        verbose_name=_("Статус")
    )
    is_guest = models.BooleanField(
        default=False,
        verbose_name=_("Гостевое пространство")
    )

    class Meta:
        verbose_name = _("Пространство")
        verbose_name_plural = _("Пространства")
        ordering = ["-created_at"]
        unique_together = ['title', 'author']

    def __str__(self):
        return self.title

    def clean(self):
        if self.start_date and self.end_date and self.start_date > self.end_date:
            raise ValidationError(
                _("Дата начала не может быть позже даты окончания.")
            )

    def get_all_elements(self):
        """
        Получает все элементы, принадлежащие этому пространству,
        независимо от того, находятся ли они на уровне пространства или на страницах.
        """
        elements = []
        for subclass in Element.__subclasses__():
            elements.extend(subclass.objects.filter(workspace=self))
        return elements


class Page(models.Model):
    """
    Модель для страниц внутри пространств.
    """
    space = models.ForeignKey(
        'Workspace',
        on_delete=models.CASCADE,
        related_name='pages',
        verbose_name=_("Пространство")
    )
    parent_page = models.ForeignKey(
        'self',
        on_delete=models.CASCADE,
        related_name='subpages',
        blank=True,
        null=True,
        verbose_name=_('Родительская страница')
    )
    title = models.CharField(
        max_length=255,
        verbose_name=_("Название")
    )
    created_at = models.DateTimeField(
        auto_now_add=True,
        verbose_name=_("Дата создания")
    )
    icon = models.ImageField(
        upload_to='page_icons/',
        blank=True,
        null=True,
        verbose_name=_("Иконка")
    )

    class Meta:
        verbose_name = _("Страница")
        verbose_name_plural = _("Страницы")
        ordering = ["-created_at"]
        unique_together = ['space', 'title']

    def __str__(self):
        return f"{self.space.title} - {self.title}"

    def get_all_elements(self):
        """
        Получает все элементы, принадлежащие этой странице,
        включая элементы подстраниц рекурсивно.
        """
        elements = []
        for subclass in Element.__subclasses__():
            elements.extend(subclass.objects.filter(page=self))
        
        for subpage in self.subpages.all():
            elements.extend(subpage.get_all_elements())
        
        return elements

    def get_full_path(self):
        """
        Возвращает полный путь к странице через все родительские страницы.
        """
        path = [self.title]
        parent = self.parent_page
        while parent:
            path.insert(0, parent.title)
            parent = parent.parent_page
        return " > ".join(path)


class Element(models.Model):
    """
    Базовая модель для всех типов элементов.
    """
    ELEMENT_TYPES = (
        ('image', 'Image'),
        ('file', 'File'),
        ('checkbox', 'Checkbox'),
        ('text', 'Text'),
        ('link', 'Link'),
    )

    element_type = models.CharField(
        max_length=20,
        choices=ELEMENT_TYPES,
        default='text'
    )

    workspace = models.ForeignKey(
        'Workspace',
        on_delete=models.CASCADE,
        related_name='%(class)s_elements',
        verbose_name=_("Пространство"),
        blank=True,
        null=True
    )

    page = models.ForeignKey(
        'Page',
        on_delete=models.CASCADE,
        related_name='%(class)s_elements',
        verbose_name=_("Страница"),
        blank=True,
        null=True
    )

    created_at = models.DateTimeField(
        auto_now_add=True,
        verbose_name=_("Дата создания")
    )

    class Meta:
        abstract = True
        ordering = ["-created_at"]

    def __str__(self):
        return f"{self._meta.verbose_name} - {self.workspace or self.page}"

    def clean(self):
        """
        Проверка, что элемент связан либо с пространством, либо со страницей,
        но не с обоими одновременно.
        """
        if self.workspace and self.page:
            raise ValidationError(
                _("Элемент должен быть связан либо с пространством, либо со страницей, но не с обоими одновременно.")
            )
        if not self.workspace and not self.page:
            raise ValidationError(
                _("Элемент должен быть связан либо с пространством, либо со страницей.")
            )


class ImageElement(Element):
    """
    Модель для элементов типа "Изображение".
    """
    image = models.ImageField(
        upload_to='element_images/',
        verbose_name=_("Изображение")
    )

    def save(self, *args, **kwargs):
        self.element_type = 'image'
        super().save(*args, **kwargs)

    class Meta:
        verbose_name = _("Изображение")
        verbose_name_plural = _("Изображения")


class FileElement(Element):
    """
    Модель для элементов типа "Файл".
    """
    file = models.FileField(
        upload_to='element_files/',
        verbose_name=_("Файл")
    )

    def save(self, *args, **kwargs):
        self.element_type = 'file'
        super().save(*args, **kwargs)

    class Meta:
        verbose_name = _("Файл")
        verbose_name_plural = _("Файлы")


class CheckboxElement(Element):
    """
    Модель для элементов типа "Чекбокс".
    """
    text = models.CharField(
        max_length=255,
        verbose_name=_("Текст")
    )
    is_checked = models.BooleanField(
        default=False,
        verbose_name=_("Отмечено")
    )

    def save(self, *args, **kwargs):
        self.element_type = 'checkbox'
        super().save(*args, **kwargs)

    class Meta:
        verbose_name = _("Чекбокс")
        verbose_name_plural = _("Чекбоксы")


class TextElement(Element):
    """
    Модель для элементов типа "Текстовое поле".
    """
    content = models.TextField(
        verbose_name=_("Содержимое")
    )

    def save(self, *args, **kwargs):
        self.element_type = 'text'
        super().save(*args, **kwargs)

    class Meta:
        verbose_name = _("Текстовое поле")
        verbose_name_plural = _("Текстовые поля")


class LinkElement(Element):
    """
    Модель для элементов типа "Ссылка на страницу".
    """
    linked_page = models.ForeignKey(
        'Page',
        on_delete=models.CASCADE,
        related_name='linked_elements',
        verbose_name=_("Связанная страница")
    )

    def save(self, *args, **kwargs):
        self.element_type = 'link'
        super().save(*args, **kwargs)

    class Meta:
        verbose_name = _("Ссылка на страницу")
        verbose_name_plural = _("Ссылки на страницы")

    def clean(self):
        super().clean()
        if self.linked_page:
            if self.workspace and self.linked_page.space != self.workspace:
                raise ValidationError(
                    _("Ссылка должна указывать на страницу в том же пространстве.")
                )
            elif self.page and self.linked_page.space != self.page.space:
                raise ValidationError(
                    _("Ссылка должна указывать на страницу в том же пространстве.")
                )
