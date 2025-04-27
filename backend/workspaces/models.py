from django.db import models
from django.utils.translation import gettext_lazy as _
from django.core.exceptions import ValidationError
from django.conf import settings
from django.utils.text import slugify
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
        verbose_name=_("Название")
    )
    author = models.ForeignKey(
        settings.AUTH_USER_MODEL,
        on_delete=models.CASCADE,
        related_name='spaces',
        verbose_name=_("Автор")
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

    class Meta:
        verbose_name = _("Пространство")
        verbose_name_plural = _("Пространства")
        ordering = ["-created_at"]

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
    link = models.SlugField(
        unique=True,
        verbose_name=_("Ссылка")
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

    def __str__(self):
        return self.title

    def save(self, *args, **kwargs):
        if not self.link:
            base_slug = slugify(self.title)
            slug = base_slug
            counter = 1
            while Page.objects.filter(link=slug).exists():
                slug = f"{base_slug}-{counter}"
                counter += 1
            self.link = slug

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
    Базовая модель для элементов на страницах.
    """
    ELEMENT_TYPES = [
        ('image', _('Изображение')),
        ('file', _('Файл')),
        ('checkbox', _('Чекбокс')),
        ('text', _('Текстовое поле')),
        ('link', _('Ссылка на страницу')),
    ]

    page = models.ForeignKey(
        'Page',
        on_delete=models.CASCADE,
        related_name='elements',
        verbose_name=_("Страница")
    )
    element_type = models.CharField(
        max_length=20,
        choices=ELEMENT_TYPES,
        verbose_name=_("Тип элемента")
    )
    created_at = models.DateTimeField(
        auto_now_add=True,
        verbose_name=_("Дата создания")
    )

    class Meta:
        verbose_name = _("Элемент")
        verbose_name_plural = _("Элементы")
        ordering = ["-created_at"]

    def __str__(self):
        return f"{self.get_element_type_display()} - {self.page.title}"


class ImageElement(models.Model):
    """
    Модель для элементов типа "Изображение".
    """
    element = models.OneToOneField(
        Element,
        on_delete=models.CASCADE,
        related_name='imageelement',
        verbose_name=_("Элемент")
    )
    image = models.ImageField(
        upload_to='element_images/',
        verbose_name=_("Изображение")
    )

    class Meta:
        verbose_name = _("Изображение")
        verbose_name_plural = _("Изображения")

    def __str__(self):
        return f"Изображение - {self.element.page.title}"


class FileElement(models.Model):
    """
    Модель для элементов типа "Файл".
    """
    element = models.OneToOneField(
        Element,
        on_delete=models.CASCADE,
        related_name='fileelement',
        verbose_name=_("Элемент")
    )
    file = models.FileField(
        upload_to='element_files/',
        verbose_name=_("Файл")
    )

    class Meta:
        verbose_name = _("Файл")
        verbose_name_plural = _("Файлы")

    def __str__(self):
        return f"Файл - {self.element.page.title}"


class CheckboxElement(models.Model):
    """
    Модель для элементов типа "Чекбокс".
    """
    element = models.OneToOneField(
        Element,
        on_delete=models.CASCADE,
        related_name='checkboxelement',
        verbose_name=_("Элемент")
    )
    text = models.CharField(
        max_length=255,
        verbose_name=_("Текст")
    )
    is_checked = models.BooleanField(
        default=False,
        verbose_name=_("Отмечено")
    )

    class Meta:
        verbose_name = _("Чекбокс")
        verbose_name_plural = _("Чекбоксы")

    def __str__(self):
        return f"Чекбокс - {self.element.page.title}"


class TextElement(models.Model):
    """
    Модель для элементов типа "Текстовое поле".
    """
    element = models.OneToOneField(
        Element,
        on_delete=models.CASCADE,
        related_name='textelement',
        verbose_name=_("Элемент")
    )
    content = models.TextField(
        verbose_name=_("Содержимое")
    )

    class Meta:
        verbose_name = _("Текстовое поле")
        verbose_name_plural = _("Текстовые поля")

    def __str__(self):
        return f"Текстовое поле - {self.element.page.title}"


class LinkElement(models.Model):
    """
    Модель для элементов типа "Ссылка на страницу".
    """
    element = models.OneToOneField(
        Element,
        on_delete=models.CASCADE,
        related_name='linkelement',
        verbose_name=_("Элемент")
    )
    linked_page = models.ForeignKey(
        'Page',
        on_delete=models.CASCADE,
        related_name='linked_elements',
        verbose_name=_("Связанная страница")
    )

    class Meta:
        verbose_name = _("Ссылка на страницу")
        verbose_name_plural = _("Ссылки на страницы")

    def __str__(self):
        return f"Ссылка - {self.element.page.title}"

    def clean(self):
        if self.linked_page.space != self.element.page.space:
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
            link="main",
            is_main=True
        )
