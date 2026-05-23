from django.db import models

class Register(models.Model):
    username = models.CharField(max_length=100,null=True, blank=True)
    age = models.IntegerField(null=True, blank=True)
    GENDER_CHOICES = [('M', 'Male'),('F', 'Female'),('O', 'Other'),]
    gender = models.CharField(max_length=1, choices=GENDER_CHOICES, null=True, blank=True)
    email = models.EmailField(max_length=100,unique=True)
    phone = models.CharField(max_length=10, null=True, blank=True)
    address = models.TextField(null=True, blank=True)
    password = models.CharField(max_length=100, null=True, blank=True)
    image = models.FileField(upload_to='images/', null=True, blank=True)

class WaterReading(models.Model):
    temperature = models.FloatField()
    tds = models.FloatField()
    turbidity = models.FloatField()
    water_level = models.FloatField()
    refactor_index = models.FloatField()
    quality_status = models.CharField(max_length=20)
    active = models.BooleanField(default=True)
    created_at = models.DateTimeField(auto_now_add=True)
    def __str__(self):
        return f"RI {self.refactor_index} - {self.quality_status} ({self.created_at})"

class SystemMode(models.Model):
    live_mode = models.BooleanField(default=True)
    updated_at = models.DateTimeField(auto_now=True)
    def __str__(self):
        if self.live_mode:
            return "LIVE MODE"
        return "SINGLE MODE"

class LatestSensorData(models.Model):
    temperature = models.FloatField()
    tds = models.FloatField()
    turbidity = models.FloatField()
    water_level = models.FloatField()
    refactor_index = models.FloatField()
    quality_status = models.CharField(max_length=20)
    updated_at = models.DateTimeField(auto_now=True)