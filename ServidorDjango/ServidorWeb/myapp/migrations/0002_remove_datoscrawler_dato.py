# Generated by Django 5.1.1 on 2024-10-05 14:22

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('myapp', '0001_initial'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='datoscrawler',
            name='dato',
        ),
    ]