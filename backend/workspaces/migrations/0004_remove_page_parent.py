# Generated by Django 3.2.16 on 2025-05-27 13:08

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('workspaces', '0003_page_parent'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='page',
            name='parent',
        ),
    ]
