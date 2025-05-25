from django.db import models
from django.utils.translation import gettext_lazy as _
from django.core.exceptions import ValidationError
from django.conf import settings
from django.utils.text import slugify
from django.db.models.signals import post_save
from django.dispatch import receiver
from django.contrib.contenttypes.fields import GenericRelation
from django.contrib.auth.models import User


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
                _("Дата начала не может быть позже даты окончания."))


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
    title = models.CharField(
        max_length=255,
        verbose_name=_("Название")
    )
    is_main = models.BooleanField(
        default=False,
        verbose_name=_("Главная страница")
    )
    created_at = models.DateTimeField(
        auto_now_add=True,
        verbose_name=_("Дата создания")
    )

    class Meta:
        verbose_name = _("Страница")
        verbose_name_plural = _("Страницы")
        ordering = ["-created_at"]
        unique_together = ['space', 'title']

    def __str__(self):
        return f"{self.space.title} - {self.title}"

    def save(self, *args, **kwargs):
        if self.is_main:
            Page.objects.filter(
                space=self.space, is_main=True).update(is_main=False)
        super().save(*args, **kwargs)

    def delete(self, *args, **kwargs):
        if self.is_main:
            raise ValidationError(
                _("Нельзя удалить главную страницу. Удалите пространство."))
        super().delete(*args, **kwargs)


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
    page = models.ForeignKey(
        'Page',
        on_delete=models.CASCADE,
        related_name='%(class)s_elements',
        verbose_name=_("Страница")
    )
    created_at = models.DateTimeField(
        auto_now_add=True,
        verbose_name=_("Дата создания")
    )

    class Meta:
        abstract = True
        ordering = ["-created_at"]

    def __str__(self):
        return f"{self._meta.verbose_name} - {self.page.title}"


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
        if self.linked_page.space != self.page.space:
            raise ValidationError(
                _("Ссылка должна указывать на страницу в том же пространстве."))


@receiver(post_save, sender=Workspace)
def create_main_page(sender, instance, created, **kwargs):
    """
    Создание главной страницы при создании пространства.
    """
    if created:
        Page.objects.create(
            space=instance,
            title="Главная",
            is_main=True
        )
