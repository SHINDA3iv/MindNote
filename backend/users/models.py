from django.db import models
from django.contrib.auth.models import AbstractUser

class MindNoteUser(AbstractUser):


    class Meta:
        ordering = ["id"]
        verbose_name = "Пользователь"
        verbose_name_plural = "Пользователи"

    def __str__(self):
        return self.username
    
