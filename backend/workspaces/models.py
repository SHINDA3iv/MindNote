from django.contrib.auth import get_user_model
from django.db import models, transaction
from django.utils.text import slugify
import uuid
from django.contrib.contenttypes.fields import GenericForeignKey
from django.contrib.contenttypes.models import ContentType

MindNoteUser = get_user_model()

class Workspace(models.Model):
    name = models.CharField(max_length=32, verbose_name="Название") 
    owner = models.ForeignKey(MindNoteUser, on_delete= models.CASCADE, verbose_name="Владелец", related_name="workspaces")
    created_at = models.DateTimeField(auto_now_add=True)  
    updated_at = models.DateTimeField(auto_now=True)  

    class Meta:
        ordering = ["updated_at"]
        verbose_name = "Пространство"
        verbose_name_plural = "Пространства"

    def __str__(self):
        return self.name
    
    
class Page(models.Model):
    workspace = models.ForeignKey(Workspace, on_delete=models.CASCADE, related_name='pages')
    name = models.CharField(max_length=32)
    slug = models.SlugField(unique=True, null=True, blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)
    icon = models.ImageField(upload_to="pages/icons/", null=True, default="")
    banner = models.ImageField(upload_to="pages/banners,", null=True, default="")

    class Meta:
        ordering = ['created_at'] 
        verbose_name = "Страница"
        verbose_name_plural = "Страницы"

    def __str__(self):
        return self.name
    
    def save(self, *args, **kwargs):
        if not self.slug:
            base_slug = slugify(self.name)
            unique_slug = base_slug
            while Page.objects.filter(slug=unique_slug).exists():
                unique_slug = f"{base_slug}-{uuid.uuid4().hex[:6]}"
            self.slug = unique_slug
        super().save(*args, **kwargs)

class ContentElement(models.Model):
    page = models.ForeignKey('Page', on_delete=models.CASCADE, related_name='page_elements')
    order = models.PositiveIntegerField(default=0)
    content_type = models.ForeignKey(ContentType, on_delete=models.CASCADE)
    object_id = models.PositiveIntegerField()
    content_object = GenericForeignKey('content_type', 'object_id')

    class Meta:
        ordering = ['order']
        constraints = [
            models.UniqueConstraint(fields=['page', 'order'], name='unique_order_per_page')
        ]
        verbose_name = "Элемент"
        verbose_name_plural = "Элементы"

    def __str__(self):
        return f"Element {self.id} on page {self.page.name}"
    
    
class TextElement(ContentElement):
    content = models.TextField(null=True, blank=True)  

class ImageElement(ContentElement):
    image = models.ImageField(upload_to='images/')

class FileElement(ContentElement):
    file = models.FileField(upload_to='files/')  

class LinkElement(ContentElement):
    linked_page = models.ForeignKey(Page, on_delete=models.SET_NULL, null=True, blank=True, related_name='linked_elements') 
