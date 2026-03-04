from django.urls import path
from . import views

urlpatterns = [
    path('', views.index, name='index'),
    path('register/', views.register, name='register'),
    path('login/', views.login, name='login'),
    path('userhome/', views.userhome, name='userhome'),
    path('logout/', views.logout, name='logout'),
    # path('profile/', views.profile, name='profile'),
    # path('editprofile/', views.editprofile, name='editprofile'),

    path('api/water-data/', views.receive_water_data),
    path('api/latest-readings/', views.latest_readings),
    path('api/set-mode/', views.set_mode),
    path('api/collect-reading/', views.collect_reading),
    path('api/manual-reading/', views.manual_reading),
]
