from django.db import models

# Create your models here.


class Register(models.Model):
    username = models.CharField(max_length=100,null=True, blank=True)
    age = models.IntegerField(null=True, blank=True)
    GENDER_CHOICES = [
        ('M', 'Male'),
        ('F', 'Female'),
        ('O', 'Other'),
    ]
    gender = models.CharField(max_length=1, choices=GENDER_CHOICES, null=True, blank=True)
    email = models.EmailField(max_length=100,unique=True)
    phone = models.CharField(max_length=10, null=True, blank=True)
    address = models.TextField(null=True, blank=True)
    password = models.CharField(max_length=100, null=True, blank=True)
    image = models.FileField(upload_to='images/', null=True, blank=True)
    