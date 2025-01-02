from django.db import models

# Create your models here.

class DensityEntry(models.Model):
    density = models.IntegerField(default=0)

    class Meta:
        verbose_name_plural = "DensityEntries"

    def create():
        num = 12 - DensityEntry.objects.__len__()

        if num >= 0:
            for _ in range(num):
                DensityEntry.objects.create()

