# Generated by Django 5.1.1 on 2024-10-05 14:25

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('myapp', '0002_remove_datoscrawler_dato'),
    ]

    operations = [
        migrations.AddField(
            model_name='datoscrawler',
            name='dato',
            field=models.CharField(default='empty', max_length=200),
            preserve_default=False,
        ),
    ]
