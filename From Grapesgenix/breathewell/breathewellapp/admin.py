from django.contrib import admin
from .models import WaterReading, SystemMode, LatestSensorData


admin.site.register(WaterReading)
admin.site.register(SystemMode)
admin.site.register(LatestSensorData)