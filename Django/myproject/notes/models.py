from django.db import models

class UserSpace(models.Model):
    user_id = models.IntegerField(primary_key=True)
    space_id = models.IntegerField(primary_key=True)

class Space(models.Model):
    space_id = models.IntegerField(primary_key=True)
    name = models.CharField(max_length=255)
    page_id = models.IntegerField()

class Page(models.Model):
    page_id = models.IntegerField(primary_key=True)
    name = models.CharField(max_length=255)
    order = models.IntegerField()
    element_type = models.CharField(max_length=255)
    element_table_id = models.IntegerField()
    property_id = models.ForeignKey('PageProperty', on_delete=models.CASCADE)

class PageProperty(models.Model):
    property_id = models.IntegerField(primary_key=True)
    page = models.ForeignKey(Page, on_delete=models.CASCADE)
    icon = models.ImageField(upload_to='icons/')
    banner = models.ImageField(upload_to='banners/')
    start_date = models.DateTimeField()
    end_date = models.DateTimeField()
    has_tag = models.BooleanField()
    status_id = models.ForeignKey('Status', on_delete=models.CASCADE)
    information = models.CharField(max_length=255)

class Tag(models.Model):
    tag_id = models.IntegerField(primary_key=True)
    name = models.CharField(max_length=255)
    color = models.IntegerField()  # Hex color as int
    background = models.IntegerField()  # Hex color as int
    page_id = models.ForeignKey(Page, on_delete=models.CASCADE)

class Status(models.Model):
    status_id = models.IntegerField(primary_key=True)
    name = models.ImageField(upload_to='status_images/')

class HTMLText(models.Model):
    id = models.IntegerField(primary_key=True)
    text = models.TextField()

class Image(models.Model):
    id = models.IntegerField(primary_key=True)
    image = models.ImageField(upload_to='images/')

class File(models.Model):
    id = models.IntegerField(primary_key=True)
    file = models.FileField(upload_to='files/')
